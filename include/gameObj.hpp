#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>

enum class ObjectType
{
    Wall,
    Box,
    Door,
    Floor,
    Disguise,
    Terminal
};

class GameObject
{
private:
    sf::CircleShape shape;
    ObjectType type;

public:
    GameObject(float rad, int points, sf::Vector2f pos, sf::Color color, ObjectType t);
    virtual void draw(sf::RenderWindow &window);

    sf::FloatRect getBounds() const;
    ObjectType getType() const;
    sf::Vector2f getPosition() const;
    sf::Vector2i getGridPosition(float tileSize) const;

    void setPosition(sf::Vector2f &pos);
    void setFillColor(sf::Color color);
};