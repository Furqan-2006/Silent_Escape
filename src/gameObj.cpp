#include "gameObj.hpp"

// Constructor for the GameObject class, initializing the sprite with a texture, position, object type, and grid position.
GameObject::GameObject(const sf::Texture &tex, sf::Vector2f pos, ObjectType t, sf::Vector2i gridP, float tileSize)
    : type(t), gridPos(gridP), sprite(std::make_unique<sf::Sprite>(tex)) // Initialize members
{
    // Set sprite origin to the bottom-center based on the texture size
    sprite->setOrigin({tex.getSize().x / 2.f, (float)tex.getSize().y});

    // Convert the grid position to isometric coordinates, then adjust the position
    sf::Vector2f isoPos = toIsometric(gridP, tileSize);
    isoPos += {32.f, 32.f}; // Adjust position for rendering

    // Set the sprite's position based on the calculated isometric position
    sprite->setPosition(isoPos);

    // Define the collision box parameters (offset and size)
    float offsetX = 10.f;
    float offsetY = 40.f;
    float width = 44.f;
    float height = 24.f;

    // Initialize the collision box with the position and size based on the sprite's position and origin
    collisionBox = sf::FloatRect(
        {isoPos.x - sprite->getOrigin().x + offsetX, isoPos.y - sprite->getOrigin().y + offsetY},
        {width, height});
}

// This function draws the sprite onto the window.
void GameObject::draw(sf::RenderWindow &window)
{
    window.draw(*sprite); // Draw the sprite
    // std::cout<<"drawn\n"; // Debugging line, can be removed
}

// Getter function to return the global bounds of the sprite (used for collision or bounding box checks).
sf::FloatRect GameObject::getBounds() const
{
    return sprite->getGlobalBounds(); // Return the global bounds of the sprite
}

// Getter function to return the type of the game object (e.g., player, enemy).
ObjectType GameObject::getType() const
{
    return type; // Return the object type
}

// Getter function to return the position of the sprite.
sf::Vector2f GameObject::getPos() const
{
    return sprite->getPosition(); // Return the position of the sprite
}

// Getter function to return the grid position of the game object (used for map/grid-based games).
sf::Vector2i GameObject::getGridPosition() const
{
    return gridPos; // Return the grid position of the object
}

// Setter function to update the position of the sprite.
void GameObject::setPos(sf::Vector2f &pos)
{
    sprite->setPosition(pos); // Set the sprite's position
}

// Getter function to return the collision box (used for collision detection).
sf::FloatRect GameObject::getCollisionBox() const
{
    return collisionBox; // Return the collision box for the game object
}
