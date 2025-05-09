#include "gameObj.hpp"

GameObject::GameObject(float rad, int points, sf::Vector2f pos, sf::Color color, ObjectType t, sf::Vector2i gridP) : type(t), gridPos(gridP)
{
    shape.setPointCount(points);
    shape.setRadius(rad);
    shape.setFillColor(color);
    shape.setPosition(pos);
}
void GameObject::draw(sf::RenderWindow &window)
{
    window.draw(shape);
}

sf::FloatRect GameObject::getBounds() const
{
    return shape.getGlobalBounds();
}
ObjectType GameObject::getType() const
{
    return type;
}

sf::Vector2f GameObject::getPos() const
{
    return shape.getPosition();
}

sf::Vector2i GameObject::getGridPosition() const
{
    return gridPos;
}

void GameObject::setPos(sf::Vector2f &pos)
{
    shape.setPosition(pos);
}
void GameObject::setFillColor(sf::Color color)
{
    shape.setFillColor(color);
}
