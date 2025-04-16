#include "player.hpp"

Player::Player(float sp) : speed(sp)
{
    rect.setSize(sf::Vector2f({40.f, 40.f}));
    rect.setFillColor(sf::Color::Green);
    rect.setPosition({400, 300});
}

bool Player::checkCollision(const sf::FloatRect &otherBounds) const
{
    return rect.getGlobalBounds().findIntersection(otherBounds).has_value();
}
void Player::moveUp(const std::vector<GameObject> &obstacles)
{
    rect.move({0.f, -speed});
    std::cout << "[LOG] player moved up " << std::endl;
    for (const auto &obj : obstacles)
    {
        if (checkCollision(obj.getBounds()))
        {
            rect.move({0.f, speed});
            std::cout << "[LOG] Collision on up\n";
            break;
        }
    }
}
void Player::moveDown(const std::vector<GameObject> &obstacles)
{
    rect.move({0.f, speed});
    std::cout << "[LOG] player moved down " << std::endl;
    for (const auto &obj : obstacles)
    {
        if (checkCollision(obj.getBounds()))
        {
            rect.move({0.f, -speed});
            std::cout << "[LOG] Collision on down\n";
            break;
        }
    }
}
void Player::moveLeft(const std::vector<GameObject> &obstacles)
{
    rect.move({-speed, 0.f});
    std::cout << "[LOG] player moved left " << std::endl;
    for (const auto &obj : obstacles)
    {
        if (checkCollision(obj.getBounds()))
        {
            rect.move({speed, 0.f});
            std::cout << "[LOG] Collision on left\n";
            break;
        }
    }
}
void Player::moveRight(const std::vector<GameObject> &obstacles)
{
    rect.move({speed, 0.f});
    std::cout << "[LOG] player moved right " << std::endl;
    for (const auto &obj : obstacles)
    {
        if (checkCollision(obj.getBounds()))
        {
            rect.move({-speed, 0.f});
            std::cout << "[LOG] Collision on right\n";
            break;
        }
    }
}

void Player::disguise()
{
    std::cout << "[LOG] player is disguising " << std::endl;
}
void Player::hide()
{
    std::cout << "[LOG] player is hiding " << std::endl;
}
void Player::hack()
{
    std::cout << "[LOG] player is hacking " << std::endl;
}
void Player::draw(sf::RenderWindow &window)
{
    window.draw(rect);
}