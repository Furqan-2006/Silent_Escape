#include "player.hpp"

// Constructor to initialize the player with texture and speed
Player::Player(const sf::Texture &tex, float sp) : speed(sp), disguised(false), hidden(false), sprite(std::make_unique<sf::Sprite>(tex))
{
    // Initialize action cooldowns (hack, hide, disguise, distract)
    actionClocks["hack"];
    actionClocks["hide"];
    actionClocks["disguise"];
    actionClocks["distract"];

    // Set action cooldown durations
    setActionCooldown("hack", 10.f);    // Hack cooldown: 10 seconds
    setActionCooldown("disguise", 8.f); // Disguise cooldown: 8 seconds
    setActionCooldown("hide", 10.f);    // Hide cooldown: 10 seconds
    setActionCooldown("distract", 2.f); // Distract cooldown: 2 seconds

    // Set sprite's origin to its center and scale the sprite
    sprite->setOrigin({tex.getSize().x / 2.f, (float)tex.getSize().y});
    sprite->setScale({32.f / tex.getSize().x, 32.f / tex.getSize().y});
}

// Check if the player collides with another object based on its bounds
bool Player::checkCollision(const sf::FloatRect &otherBounds) const
{
    return sprite->getGlobalBounds().findIntersection(otherBounds).has_value();
}

// Check if the player can move by the given offset, ensuring no collisions with obstacles
bool Player::canMove(sf::Vector2f offset, const std::vector<GameObject> &obstacles)
{
    sprite->move(offset); // Move the player by the offset
    for (const auto &obs : obstacles)
    {
        // Use collisionBox for consistent detection
        if (checkCollision(obs.getCollisionBox())) // If collision occurs
        {
            sprite->move(-offset); // Undo the movement
            return false;          // Can't move
        }
    }
    return true; // Can move
}

// Move the player upwards if no collision is detected
void Player::moveUp(float deltaTime, const std::vector<GameObject> &obstacles)
{
    sf::Vector2f dir = toIsometricDir({0.f, -1.f}, 64.f); // Calculate direction upwards in isometric coordinates
    if (canMove(dir * speed * deltaTime, obstacles))      // Check if the player can move
    {
        std::cout << "[LOG] player moved up" << std::endl;
    }
    else
    {
        std::cout << "[LOG] Collision on up\n";
    }
}

// Move the player downwards if no collision is detected
void Player::moveDown(float deltaTime, const std::vector<GameObject> &obstacles)
{
    sf::Vector2f dir = toIsometricDir({0.f, 1.f}, 64.f); // Calculate direction downwards in isometric coordinates
    if (canMove(dir * speed * deltaTime, obstacles))     // Check if the player can move
    {
        std::cout << "[LOG] player moved down" << std::endl;
    }
    else
    {
        std::cout << "[LOG] Collision on down\n";
    }
}

// Move the player to the left if no collision is detected
void Player::moveLeft(float deltaTime, const std::vector<GameObject> &obstacles)
{
    sf::Vector2f dir = toIsometricDir({-1.f, 0.f}, 64.f); // Calculate direction to the left in isometric coordinates
    if (canMove(dir * speed * deltaTime, obstacles))      // Check if the player can move
    {
        std::cout << "[LOG] player moved left" << std::endl;
    }
    else
    {
        std::cout << "[LOG] Collision on left\n";
    }
}

// Move the player to the right if no collision is detected
void Player::moveRight(float deltaTime, const std::vector<GameObject> &obstacles)
{
    sf::Vector2f dir = toIsometricDir({1.f, 0.f}, 64.f); // Calculate direction to the right in isometric coordinates
    if (canMove(dir * speed * deltaTime, obstacles))     // Check if the player can move
    {
        std::cout << "[LOG] player moved right" << std::endl;
    }
    else
    {
        std::cout << "[LOG] Collision on right\n";
    }
}

// Set the disguise status of the player
void Player::setDisguised(bool value)
{
    disguised = value;
}

// Set the hidden status of the player
void Player::setHidden(bool value)
{
    hidden = value;
}

// Get the disguise status of the player
bool Player::isDisguised() const { return disguised; }

// Get the hidden status of the player
bool Player::isHidden() const { return hidden; }

// Make the player disguise (if not on cooldown)
void Player::disguise()
{
    if (isOnCooldown("disguise"))
    {
        std::cout << "[LOG] disguise on cooldown.\n";
        return;
    }

    disguised = true; // Set player as disguised
    std::cout << "[LOG] player is now disguised " << std::endl;
    actionClocks["disguise"].restart(); // Restart disguise cooldown
}

// Make the player hide (if not on cooldown)
void Player::hide()
{
    if (isOnCooldown("hide"))
    {
        hidden = false; // Set player as not hidden if hiding is on cooldown
        std::cout << "[LOG] hiding on cooldown.\n";
        return;
    }
    hidden = true; // Set player as hidden
    std::cout << "[LOG] player is hidden " << std::endl;
    actionClocks["hide"].restart(); // Restart hide cooldown
}

// Make the player hack (if not on cooldown)
void Player::hack()
{
    if (isOnCooldown("hack"))
    {
        std::cout << "[LOG] hack on cooldown.\n";
        return;
    }
    std::cout << "[LOG] player is hacking..." << std::endl;
    actionClocks["hack"].restart(); // Restart hack cooldown
}

// Make the player distract (if not on cooldown)
void Player::distract() // throw distractions
{
    if (isOnCooldown("distract"))
    {
        std::cout << "[LOG] distract on cooldown\n";
        return;
    }
    std::cout << "[LOG] Player is distracting guard";
    Distraction thing(getPos(), 100.f); // Create a distraction at player's position
    distractions.push_back(thing);      // Add distraction to the list
    actionClocks["distract"].restart(); // Restart distract cooldown
}

// Get the list of active distractions
const std::vector<Distraction> &Player::getDistractions() const
{
    return distractions;
}

// Cleanup expired distractions
void Player::cleanupDistractions()
{
    distractions.erase(
        std::remove_if(distractions.begin(), distractions.end(), [](const Distraction &d)
                       { return d.isExpired(); }), // Remove expired distractions
        distractions.end());
}

// Update player state (check cooldowns, update disguise, hiding, check if level is won)
void Player::update(GameState &gameState)
{
    if (disguised && actionClocks["disguise"].getElapsedTime().asSeconds() > 10.f)
    {
        disguised = false; // End disguise after 10 seconds
        std::cout << "[LOG] disguise ended\n";
    }
    if (hidden && actionClocks["hide"].getElapsedTime().asSeconds() > 10.f)
    {
        hidden = false; // End hiding after 10 seconds
        std::cout << "[LOG] hiding ended\n";
    }

    if (won()) // If the player won, change the game state
    {
        gameState = GameState::LEVEL_CLEAR;
        std::cout << "[LOG] Level Won!\n";
    }

    cleanupDistractions(); // Remove expired distractions
}

// Set the cooldown duration for an action
void Player::setActionCooldown(std::string action, float dur)
{
    actionCooldowns[action] = dur;
}

// Check if an action is on cooldown
bool Player::isOnCooldown(std::string action) const
{
    auto it = actionCooldowns.find(action);
    if (it == actionCooldowns.end())
        return false;                                                         // Return false if the action doesn't have a cooldown
    return actionClocks.at(action).getElapsedTime().asSeconds() < it->second; // Return true if the action is still on cooldown
}

// Draw the player sprite to the window
void Player::draw(sf::RenderWindow &window)
{
    window.draw(*sprite); // Draw the player sprite
}

// Set the player's position
void Player::setPos(sf::Vector2f &pos)
{
    sprite->setPosition(pos);
}

// Set the player's grid position
void Player::setGridPos(sf::Vector2i &pos)
{
    gridPosition = pos;
}

// Get the player's current position
sf::Vector2f Player::getPos() const
{
    return sprite->getPosition();
}

// Get the player's grid position
sf::Vector2i Player::getGridPosition() const
{
    return gridPosition;
}

// Get the player's bounding box for collision detection
sf::FloatRect Player::getBounds() const
{
    return sprite->getGlobalBounds();
}

// Draw a tile highlight under the player to indicate the current tile
void Player::drawTileHighlight(sf::RenderWindow &window, int tileWidth, int tileHeight)
{
    // Get player position
    sf::Vector2f pos = sprite->getPosition();

    // Convert isometric screen position to grid position
    int tileX = static_cast<int>((pos.x / (tileWidth / 2) + pos.y / (tileHeight / 2)) / 2);
    int tileY = static_cast<int>((pos.y / (tileHeight / 2) - pos.x / (tileWidth / 2)) / 2);

    // Convert grid position back to isometric screen coordinates
    float screenX = (tileX - tileY) * (tileWidth / 2.f);
    float screenY = (tileX + tileY) * (tileHeight / 2.f);

    // Draw a semi-transparent rectangle or diamond to highlight the tile under the player
    sf::ConvexShape diamond;
    diamond.setPointCount(4);
    diamond.setPoint(0, sf::Vector2f(screenX, screenY + tileHeight / 2.f));             // left
    diamond.setPoint(1, sf::Vector2f(screenX + tileWidth / 2.f, screenY));              // top
    diamond.setPoint(2, sf::Vector2f(screenX + tileWidth, screenY + tileHeight / 2.f)); // right
    diamond.setPoint(3, sf::Vector2f(screenX + tileWidth / 2.f, screenY + tileHeight)); // bottom

    diamond.setFillColor(sf::Color(255, 255, 0, 100)); // Yellow with transparency
    window.draw(diamond);                              // Draw the highlight
}

// Check if the player has reached the goal to win the level
bool Player::won()
{
    sf::Vector2f playerPos = getPos();

    // If the player is close enough to the goal position, they win
    if (std::hypot(playerPos.x - goalPos.x, playerPos.y - goalPos.y) < 10.f)
    {
        return true;
    }
    return false; // Otherwise, the player has not won yet
}
