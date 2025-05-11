#include "gameObj.hpp"

GameObject::GameObject(const sf::Texture &tex, sf::Vector2f pos, ObjectType t, sf::Vector2i gridP, float tileSize)
    : type(t), gridPos(gridP), sprite(std::make_unique<sf::Sprite>(tex)) // Initialize members
{
    // Set sprite texture
    // sprite->setTexture(tex);

    // Set sprite origin (e.g., bottom-center for isometric)
    sprite->setOrigin({tex.getSize().x / 2.f, (float)tex.getSize().y});

    sf::Vector2f isoPos = toIsometric(gridP, tileSize);
    isoPos += {32.f, 32.f};
    sprite->setPosition(isoPos);

    float offsetX = 10.f;
    float offsetY = 40.f;
    float width = 44.f;
    float height = 24.f;

    collisionBox = sf::FloatRect(
        {isoPos.x - sprite->getOrigin().x + offsetX, isoPos.y - sprite->getOrigin().y + offsetY},
        {width, height});
}

void GameObject::draw(sf::RenderWindow &window)
{
    window.draw(*sprite);
    // std::cout<<"drawn\n";
}

sf::FloatRect GameObject::getBounds() const
{
    return sprite->getGlobalBounds();
}
ObjectType GameObject::getType() const
{
    return type;
}

sf::Vector2f GameObject::getPos() const
{
    return sprite->getPosition();
}

sf::Vector2i GameObject::getGridPosition() const
{
    return gridPos;
}

void GameObject::setPos(sf::Vector2f &pos)
{
    sprite->setPosition(pos);
}
sf::FloatRect GameObject::getCollisionBox() const
{
    return collisionBox;
}
