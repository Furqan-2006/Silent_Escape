#include "gameObj.hpp"

GameObject::GameObject(float rad, int points, sf::Vector2f pos, sf::Color color, ObjectType t) : type(t)
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