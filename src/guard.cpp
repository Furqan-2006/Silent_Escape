#include "guard.hpp"
#include <SFML/System.hpp>

Guard::Guard()
{
    facingDir = velocity;
    circle.setRadius(20.f);
    circle.setFillColor(sf::Color::Cyan);
    circle.setPosition({400, 180});
    state = GuardState::Patrolling;
    velocity = {0.2, 0.f};
}

bool Guard::checkCollision(const sf::FloatRect &otherBounds) const
{
    return circle.getGlobalBounds().findIntersection(otherBounds).has_value();
}

void Guard::patrol(const std::vector<GameObject> &obstacles)
{
    circle.move(velocity);

    sf::Vector2f pos = circle.getPosition();
    for (const auto &ob : obstacles)
    {
        if (checkCollision(ob.getBounds()))
        {
            circle.move(-velocity);
            velocity *= -1.f;
        }
    }

    if (pos.x < 100 || pos.x > 500)
    {
        velocity.x *= -1;
        std::cout << "[LOG] Guard changed direction while patrolling" << std::endl;
    }

    if (velocity != sf::Vector2f(0.f, 0.f))
    {
        facingDir = velocity;
        float len = std::sqrt((facingDir.x * facingDir.x) + (facingDir.y * facingDir.y));
        if (len != 0.f)
            facingDir /= len;
    }

    std::cout << "[LOG] Guard is patrolling" << std::endl;
}

void Guard::alert()
{
    std::cout << "[LOG] Guard is alerted!" << std::endl;
}

void Guard::chase(const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles)
{
    std::cout << "[LOG] Guard is chasing!" << std::endl;

    sf::Vector2f current = circle.getPosition();
    sf::Vector2f direction = playerPos - current;

    float length = std::sqrt((direction.x * direction.x) + (direction.y * direction.y));
    if (length != 0)
    {
        direction /= length;
    }

    float speed = 0.2;
    sf::Vector2f movement = direction * speed;
    circle.move(movement);

    for (const auto &obj : obstacles)
    {
        if (checkCollision(obj.getBounds()))
        {
            circle.move(-movement);
            std::cout << "[LOG] Guard path blocked while chasing\n";
            break;
        }
    }
}

void Guard::capture(GameState &gameState)
{
    std::cout << "[LOG] Guard captured the player!" << std::endl;
    circle.setFillColor(sf::Color::White);
    sf::sleep(sf::seconds(3));
    gameState = GameState::GAME_OVER;
}

bool Guard::canSeePlayer(const sf::Vector2f &playerPos)
{
    sf::Vector2f toPlayer = playerPos - circle.getPosition();
    float distance = std::sqrt((toPlayer.x * toPlayer.x) + (toPlayer.y * toPlayer.y));

    if (distance > viewDistance)
    {
        return false;
    }

    if (distance != 0)
    {
        toPlayer /= distance;
    }
    float dot = (facingDir.x * toPlayer.x) + (facingDir.y * toPlayer.y);
    float angle = std::acos(dot) * 180.f / PI;

    return angle <= (fieldOfView / 2.f);
}

void Guard::drawSightCone(sf::RenderWindow &window)
{
    if (!sightCone)
        return;

    const int segments = 30;
    float angledRad = fieldOfView * PI / 180.f;
    float startAngle = std::atan2(facingDir.y, facingDir.x) - angledRad / 2.f;

    sf::VertexArray cone(sf::PrimitiveType::TriangleFan, segments + 2);
    sf::Vector2f origin = circle.getPosition() + sf::Vector2f(circle.getRadius(), circle.getRadius());

    cone[0].position = origin;
    cone[0].color = sightColor;

    for (int i = 0; i <= segments; i++)
    {
        float theta = startAngle + (i / static_cast<float>(segments)) * angledRad;
        sf::Vector2f point = origin + sf::Vector2f(std::cos(theta), std::sin(theta)) * viewDistance;
        cone[i + 1].position = point;
        cone[i + 1].color = sightColor;
    }
    window.draw(cone);
}

void Guard::update(Player &player, const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles, GameState &gameState)
{
    float dist = std::hypot(playerPos.x - circle.getPosition().x, playerPos.y - circle.getPosition().y);
    bool seesPlayer = canSeePlayer(playerPos);

    switch (state)
    {
    case GuardState::Patrolling:
        if (seesPlayer)
        {
            std::cout << "[FSM] switching to alerted from patrolling.\n";
            state = GuardState::Alerted;
            circle.setFillColor(sf::Color::Yellow);
        }
        else
        {
            patrol(obstacles);
        }
        break;
    case GuardState::Alerted:
        if (!seesPlayer)
        {
            std::cout << "[FSM] lost player! reverting to Patrolling\n";
            state = GuardState::Patrolling;
            circle.setFillColor(sf::Color::Cyan);
        }
        else if (dist <= 100.f)
        {
            std::cout << "[FSM] switching to chasing from alerted.\n";
            state = GuardState::Chasing;
            circle.setFillColor(sf::Color::Magenta);
        }
    case GuardState::Chasing:
        if (!seesPlayer)
        {
            std::cout << "[FSM] lost player! reverting to Patrolling\n";
            state = GuardState::Patrolling;
            circle.setFillColor(sf::Color::Cyan);
        }
        else
        {
            if (circle.getGlobalBounds().findIntersection(player.getBounds()).has_value())
            {
                capture(gameState);
            }
            else
            {
                chase(playerPos, obstacles);
            }
        }

        break;

    default:
        break;
    }
}

void Guard::draw(sf::RenderWindow &window)
{
    window.draw(circle);
}

sf::FloatRect Guard::getBounds() const
{
    return circle.getGlobalBounds();
}
