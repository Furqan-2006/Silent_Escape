#include "guard.hpp"
#include <SFML/System.hpp>

Guard::Guard()
{
    circle.setRadius(10.f);
    circle.setFillColor(sf::Color::Cyan);
    state = GuardState::Patrolling;
    velocity = {1.0f, 0.f};
    facingDir = velocity;

    lastSeenMarker.setRadius(5.f);
    lastSeenMarker.setFillColor(sf::Color::Red);
    lastSeenMarker.setOrigin({5.f, 5.f}); // Center the marker
}
Guard::Guard(const sf::Vector2f &startPos) : Guard()
{
    setPosition(startPos);
}
bool Guard::checkCollision(const sf::FloatRect &otherBounds) const
{
    return circle.getGlobalBounds().findIntersection(otherBounds).has_value();
}

void Guard::patrol(const std::vector<GameObject> &obstacles)
{
    viewDistance = 100.f;
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
void Guard::chase(const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles, PathFinder &pathfinder)
{
    viewDistance = 115.f;
    state = GuardState::Chasing;
    circle.setFillColor(sf::Color::Magenta);
    std::cout << "[LOG] Guard is chasing with pathfinding!\n";

    // Get path using A*
    std::vector<sf::Vector2f> path = pathfinder.findPath(circle.getPosition(), playerPos);

    if (!path.empty())
    {
        // Move a small step toward the next node
        sf::Vector2f direction = path[0] - circle.getPosition();
        float length = std::hypot(direction.x, direction.y);
        if (length != 0)
        {
            direction /= length;
            facingDir = direction;
        }

        float speed = 1.f;
        sf::Vector2f movement = direction * speed;
        circle.move(movement);

        // Collision with obstacles
        for (const auto &obj : obstacles)
        {
            if (checkCollision(obj.getBounds()))
            {
                circle.move(-movement);
                std::cout << "[LOG] Guard path blocked during chase\n";
                break;
            }
        }
    }
}
void Guard::search(const std::vector<GameObject> &obstacles, PathFinder &pathfinder, const sf::Vector2f &lastPlayerPos, float &tileSize)
{
    switch (currentPhase)
    {
    case SearchPhase::LookAround:
        if (!searchClockStarted)
        {
            searchClock.restart();
            searchClockStarted = true;
            std::cout << "[SEARCH] Look Around started\n";
        }

        facingDir = {
            std::cos(searchClock.getElapsedTime().asSeconds() * 2),
            std::sin(searchClock.getElapsedTime().asSeconds() * 2)};

        if (searchClock.getElapsedTime().asSeconds() > 2.f)
        {
            currentPhase = SearchPhase::Wander;
            searchClock.restart();
            searchClockStarted = false;
            std::cout << "[SEARCH] Switching to Wander\n";
        }
        break;
    case SearchPhase::Wander:
    {
        if (!searchClockStarted)
        {
            searchClock.restart();
            searchClockStarted = true;
            std::cout << "[SEARCH] Wandering started\n";

            wanderPath = pathfinder.findPath(circle.getPosition(), lastKnownPlayerPosition);
            currentWanderPathIndex = 0;
            generatedInitialWanderPath = true;
        }

        if (generatedInitialWanderPath && currentWanderPathIndex < wanderPath.size())
        {
            sf::Vector2f target = wanderPath[currentWanderPathIndex];
            sf::Vector2f direction = target - circle.getPosition();
            float length = std::hypot(direction.x, direction.y);

            if (length < 2.f)
            {
                currentWanderPathIndex++;
            }
            else
            {
                direction /= length;
                facingDir = direction;
                circle.move(direction * 1.f);

                for (const auto &obj : obstacles)
                {
                    if (checkCollision(obj.getBounds()))
                    {
                        circle.move(-direction * 1.f);
                        break;
                    }
                }
            }
        }
        else if (generatedInitialWanderPath)
        {

            if (wanderTargets.empty())
            {
                wanderTargets.clear();
                currentWanderIndex = 0;

                for (int i = 0; i < 4; i++)
                {
                    float offSetX = static_cast<float>((rand() % 81) - 40);
                    float offSetY = static_cast<float>((rand() % 81) - 40);
                    sf::Vector2f candidate = lastKnownPlayerPosition + sf::Vector2f(offSetX, offSetY);
                    sf::Vector2i gridCandidate = {
                        static_cast<int>(candidate.x / tileSize),
                        static_cast<int>(candidate.y / tileSize)};

                    if (pathfinder.isWalkable(gridCandidate.y, gridCandidate.x))
                    {
                        wanderTargets.push_back(candidate);
                    }
                }
            }

            if (currentWanderIndex < wanderTargets.size())
            {
                sf::Vector2f target = wanderTargets[currentWanderIndex];
                sf::Vector2f direction = target - circle.getPosition();
                float length = std::hypot(direction.x, direction.y);

                if (length < 2.f)
                {
                    if (!isPausingAtTarget)
                    {
                        wanderPauseClock.restart();
                        isPausingAtTarget = true;
                        std::cout << "[SEARCH] Reached wander point. Pausing to look around.\n";
                    }

                    facingDir = sf::Vector2f(std::cos(wanderPauseClock.getElapsedTime().asSeconds() * 2),
                                             std::sin(wanderPauseClock.getElapsedTime().asSeconds() * 2));

                    if (wanderPauseClock.getElapsedTime().asSeconds() >= pauseDurationAtTarget)
                    {
                        currentWanderTarget++;
                        isPausingAtTarget = false;
                    }
                }
                else
                {
                    direction /= length;
                    facingDir = direction;
                    circle.move(direction * 1.f);

                    for (const auto &obj : obstacles)
                    {
                        if (checkCollision(obj.getBounds()))
                        {
                            circle.move(-direction * 1.f);
                            break;
                        }
                    }
                }
            }
        }

        if (searchClock.getElapsedTime().asSeconds() > searchDuration)
        {
            std::cout << "[SEARCH] Switching to ToPatrol\n";
            currentPhase = SearchPhase::ReturnToPatrol;
            searchClock.restart();
            searchClockStarted = false;
            generatedInitialWanderPath = false;
            wanderTargets.clear();
        }

        break;
    }

    case SearchPhase::ReturnToPatrol:
    {
        std::cout << "[SEARCH] Returnning to Patrol--1\n";
        std::vector<sf::Vector2f> path = pathfinder.findPath(circle.getPosition(), lastKnownPlayerPosition);

        if (!path.empty())
        {
            std::cout << "[SEARCH] Returnning to Patrol--2\n";

            sf::Vector2f direction = path[0] - circle.getPosition();
            float length = std::hypot(direction.x, direction.y);
            if (length != 0.f)
                direction /= length;

            facingDir = direction;
            circle.move(direction * 1.f);

            for (const auto &obj : obstacles)
            {
                if (checkCollision(obj.getBounds()))
                {
                    circle.move(-direction * 1.f);
                    break;
                }
            }
        }

        if (std::hypot(circle.getPosition().x - initialPosition.x,
                       circle.getPosition().y - initialPosition.y) < 50.f)
        {
            std::cout << "[SEARCH] Back to Patrolling\n";
            state = GuardState::Patrolling;
            circle.setFillColor(sf::Color::Cyan);
            currentPhase = SearchPhase::LookAround;
            searchClockStarted = false;
        }
        else
        {
            state = GuardState::Patrolling;
            circle.setFillColor(sf::Color::Cyan);
            currentPhase = SearchPhase::LookAround;
            searchClockStarted = false;
        }

        break;
    }
    }
}

void Guard::capture(GameState &gameState)
{
    std::cout << "[LOG] Guard captured the player!" << std::endl;
    circle.setFillColor(sf::Color::White);
    // sf::sleep(sf::seconds(3));
    gameState = GameState::GAME_OVER;
}

bool Guard::canSeePlayer(const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles)
{
    sf::Vector2f toPlayer = playerPos - circle.getPosition();
    float distance = std::hypot(toPlayer.x, toPlayer.y);

    if (distance > viewDistance)
        return false;

    sf::Vector2f dir = toPlayer / distance;
    sf::Vector2f current = circle.getPosition();

    for (float t = 0.f; t < distance; t += 5.f)
    {
        sf::Vector2f point = current + dir * t;
        sf::FloatRect pointRect(point, {2.f, 2.f});

        for (const auto &ob : obstacles)
        {
            if (ob.getType() == ObjectType::Wall || ob.getType() == ObjectType::Box)
            {
                if (ob.getBounds().findIntersection(pointRect).has_value())
                    return false;
            }
        }
    }

    if (distance != 0)
        facingDir = dir;
    float dot = std::clamp(facingDir.x * dir.x + facingDir.y * dir.y, -1.f, 1.f);
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

void Guard::update(Player &player, const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles, GameState &gameState, PathFinder &pathfinder, sf::RenderWindow &window, float &tileSize)
{
    float dist = std::hypot(playerPos.x - circle.getPosition().x, playerPos.y - circle.getPosition().y);
    bool seesPlayer = canSeePlayer(playerPos, obstacles);

    switch (state)
    {
    case GuardState::Patrolling:
    {
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
    }
    case GuardState::Alerted:
    {
        if (!seesPlayer)
        {
            // If the player disappears mid-alert, forget about alert
            std::cout << "[FSM] lost player during alert! switching to Searching\n";
            lastKnownPlayerPosition = playerPos;
            lastSeenMarker.setPosition(lastKnownPlayerPosition);

            currentPhase = SearchPhase::LookAround;
            searchClockStarted = false;
            alertClockStarted = false;
            state = GuardState::Searching;
            circle.setFillColor(sf::Color::Red);
            break;
        }

        if (!alertClockStarted)
        {
            alertClock.restart();
            alertClockStarted = true;
            std::cout << "[FSM] Guard is alerted and pausing before chasing.\n";
        }

        if (alertClock.getElapsedTime().asSeconds() >= alertDuration)
        {
            std::cout << "[FSM] Alert duration passed. Switching to Chasing.\n";
            alertClockStarted = false;
            state = GuardState::Chasing;
            circle.setFillColor(sf::Color::Magenta);
        }
        break;
    }
    case GuardState::Chasing:
    {
        if (!seesPlayer)
        {
            std::cout << "[FSM] lost player! Staying alerted\n";
            lastKnownPlayerPosition = playerPos;
            lastSeenMarker.setPosition(lastKnownPlayerPosition);

            currentPhase = SearchPhase::LookAround;
            searchClockStarted = false;

            state = GuardState::Searching;
            circle.setFillColor(sf::Color::Red);
        }
        else
        {
            if (circle.getGlobalBounds().findIntersection(player.getBounds()).has_value())
            {
                capture(gameState);
            }
            else
            {
                chase(playerPos, obstacles, pathfinder);
            }
        }
        break;
    }
    case GuardState::Searching:
        search(obstacles, pathfinder, playerPos, tileSize);
        if (canSeePlayer(playerPos, obstacles))
        {
            std::cout << "[FSM] Player found again! switching to Alerted\n";
            state = GuardState::Alerted;
            alertClock.restart();
            circle.setFillColor(sf::Color::Yellow);
        }

        break;
    }
}

void Guard::draw(sf::RenderWindow &window)
{
    window.draw(circle);
    window.draw(lastSeenMarker);
}

void Guard::setPosition(const sf::Vector2f &position)
{
    circle.setPosition(position);
    initialPosition = position;
}

void Guard::setVelocity(const sf::Vector2f &dir)
{
    velocity = dir;
    if (velocity != sf::Vector2f({0.f, 0.f}))
    {
        facingDir = velocity / std::hypot(velocity.x, velocity.y);
    }
}

sf::FloatRect Guard::getBounds() const
{
    return circle.getGlobalBounds();
}