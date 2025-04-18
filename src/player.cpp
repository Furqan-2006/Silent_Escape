#include "player.hpp"

Player::Player(float sp) : speed(sp), disguised(false), hidden(false)
{
    rect.setSize(sf::Vector2f({40.f, 40.f}));
    rect.setFillColor(sf::Color::Green);
    rect.setPosition({400, 500});
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

void Player::setDisguised(bool value)
{
    disguised = value;
    if (!value && !disguised)
    {
        rect.setFillColor(sf::Color::Green);
    }
}
void Player::setHidden(bool value)
{
    hidden = value;
    if (!value && !hidden)
    {
        rect.setFillColor(sf::Color::Green);
    }
}

bool Player::isDisguised() const { return disguised; }
bool Player::isHidden() const { return hidden; }

void Player::disguise()
{
    disguised = true;
    rect.setFillColor(sf::Color::Cyan);
    std::cout << "[LOG] player is now disguised " << std::endl;
}
void Player::hide()
{
    hidden = true;
    rect.setFillColor(sf::Color(128, 0, 255));
    std::cout << "[LOG] player is hidden " << std::endl;
}
void Player::hack()
{
    std::cout << "[LOG] player is hacking..." << std::endl;
}
void Player::draw(sf::RenderWindow &window)
{
    window.draw(rect);
}
void Player::setPosition(sf::Vector2f pos)
{
    rect.setPosition(pos);
}
sf::Vector2f Player::getPosition() const
{
    return rect.getPosition();
}
sf::FloatRect Player::getBounds() const
{
    return rect.getGlobalBounds();
}