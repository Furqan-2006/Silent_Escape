#include "player.hpp"

Player::Player(float sp) : speed(sp), disguised(false), hidden(false)
{
    rect.setSize(sf::Vector2f({10.f, 10.f}));
    rect.setFillColor(sf::Color::Green);
    rect.setPosition({400, 500});

    actionClocks["hack"];
    actionClocks["hide"];
    actionClocks["disguise"];
    actionClocks["distract"];

    setActionCooldown("hack", 10.f);
    setActionCooldown("disguise", 8.f);
    setActionCooldown("hide", 10.f);
    setActionCooldown("distract", 2.f);
}

bool Player::checkCollision(const sf::FloatRect &otherBounds) const
{
    return rect.getGlobalBounds().findIntersection(otherBounds).has_value();
}
bool Player::canMove(sf::Vector2f offset, const std::vector<GameObject> &obstacles)
{
    rect.move(offset);
    for (const auto &obs : obstacles)
    {
        if (checkCollision(obs.getCollisionBox()))

        // if (checkCollision(obs.getBounds()))
        {
            rect.move(-offset);
            return false;
        }
    }
    return true;
}

void Player::moveUp(float deltaTime, const std::vector<GameObject> &obstacles)
{
    sf::Vector2f dir = toIsometricDir({0.f, -1.f}, 64.f);
    if (canMove(dir * speed * deltaTime, obstacles))
    {

        std::cout << "[LOG] player moved up" << std::endl;
    }
    else
    {
        std::cout << "[LOG] Collision on up\n";
    }
}
void Player::moveDown(float deltaTime, const std::vector<GameObject> &obstacles)
{
    sf::Vector2f dir = toIsometricDir({0.f, 1.f}, 64.f);
    if (canMove(dir * speed * deltaTime, obstacles))
    {

        std::cout << "[LOG] player moved down" << std::endl;
    }
    else
    {
        std::cout << "[LOG] Collision on down\n";
    }
}
void Player::moveLeft(float deltaTime, const std::vector<GameObject> &obstacles)
{
    sf::Vector2f dir = toIsometricDir({-1.f, 0.f}, 64.f);
    if (canMove(dir * speed * deltaTime, obstacles))
    {

        std::cout << "[LOG] player moved left" << std::endl;
    }
    else
    {
        std::cout << "[LOG] Collision on left\n";
    }
}
void Player::moveRight(float deltaTime, const std::vector<GameObject> &obstacles)
{
    sf::Vector2f dir = toIsometricDir({1.f, 0.f}, 64.f);
    if (canMove(dir * speed * deltaTime, obstacles))
    {
        std::cout << "[LOG] player moved right" << std::endl;
    }
    else
    {
        std::cout << "[LOG] Collision on right\n";
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
    if (isOnCooldown("disguise"))
    {
        std::cout << "[LOG] disguise on cooldown.\n";
        return;
    }

    disguised = true;
    rect.setFillColor(sf::Color::Cyan);
    std::cout << "[LOG] player is now disguised " << std::endl;
    actionClocks["disguise"].restart();
}
void Player::hide()
{
    if (isOnCooldown("hide"))
    {
        hidden = false;
        std::cout << "[LOG] hiding on cooldown.\n";
        return;
    }
    hidden = true;
    rect.setFillColor(sf::Color(128, 0, 255));
    std::cout << "[LOG] player is hidden " << std::endl;
    actionClocks["hide"].restart();
}
void Player::hack()
{
    if (isOnCooldown("hack"))
    {
        std::cout << "[LOG] hack on cooldown.\n";
        return;
    }
    std::cout << "[LOG] player is hacking..." << std::endl;
    actionClocks["hack"].restart();
}
void Player::distract() // throw distractions
{
    if (actionClocks["distract"].getElapsedTime().asSeconds() < actionCooldowns["distract"])
    {
        std::cout << "[LOG] distract on cooldown\n";
        return;
    }
    std::cout << "[LOG] Player is distracting guard";
    Distraction thing(getPos(), 100.f);
    distractions.push_back(thing);
    actionClocks["distract"].restart();
}
const std::vector<Distraction> &Player::getDistractions() const
{
    return distractions;
}
void Player::cleanupDistractions()
{
    distractions.erase(
        std::remove_if(distractions.begin(), distractions.end(), [](const Distraction &d)
                       { return d.isExpired(); }),
        distractions.end());
}

void Player::update()
{
    if (disguised && actionClocks["disguise"].getElapsedTime().asSeconds() > 10.f)
    {
        disguised = false;
        rect.setFillColor(sf::Color::Green);
        std::cout << "[LOG] disguise ended\n";
    }
    if (hidden && actionClocks["hide"].getElapsedTime().asSeconds() > 10.f)
    {
        hidden = false;
        rect.setFillColor(sf::Color::Green);
        std::cout << "[LOG] hiding ended\n";
    }

    cleanupDistractions();
}
void Player::setActionCooldown(std::string action, float dur)
{
    actionCooldowns[action] = dur;
}
bool Player::isOnCooldown(std::string action) const
{
    auto it = actionCooldowns.find(action);
    if (it == actionCooldowns.end())
        return false;
    return actionClocks.at(action).getElapsedTime().asSeconds() < it->second;
}
void Player::draw(sf::RenderWindow &window)
{
    window.draw(rect);
}
void Player::setPos(sf::Vector2f &pos)
{
    rect.setPosition(pos);
}
void Player::setGridPos(sf::Vector2i &pos)
{
    gridPosition = pos;
}

sf::Vector2f Player::getPos() const
{
    return rect.getPosition();
}
sf::Vector2i Player::getGridPosition() const
{
    return gridPosition;
}

sf::FloatRect Player::getBounds() const
{
    return rect.getGlobalBounds();
}

void Player::drawTileHighlight(sf::RenderWindow &window, int tileWidth, int tileHeight)
{
    // Get player position
    sf::Vector2f pos = rect.getPosition();

    // Convert isometric screen position to grid position
    int tileX = static_cast<int>((pos.x / (tileWidth / 2) + pos.y / (tileHeight / 2)) / 2);
    int tileY = static_cast<int>((pos.y / (tileHeight / 2) - pos.x / (tileWidth / 2)) / 2);

    // Convert grid position back to isometric screen coordinates
    float screenX = (tileX - tileY) * (tileWidth / 2.f);
    float screenY = (tileX + tileY) * (tileHeight / 2.f);

    // Draw a semi-transparent rectangle or diamond
    sf::ConvexShape diamond;
    diamond.setPointCount(4);
    diamond.setPoint(0, sf::Vector2f(screenX, screenY + tileHeight / 2.f));             // left
    diamond.setPoint(1, sf::Vector2f(screenX + tileWidth / 2.f, screenY));              // top
    diamond.setPoint(2, sf::Vector2f(screenX + tileWidth, screenY + tileHeight / 2.f)); // right
    diamond.setPoint(3, sf::Vector2f(screenX + tileWidth / 2.f, screenY + tileHeight)); // bottom

    diamond.setFillColor(sf::Color(255, 255, 0, 100)); // Yellow with transparency
    window.draw(diamond);
}
