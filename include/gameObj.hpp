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
    sf::Vector2i gridPos;

public:
    GameObject(float rad, int points, sf::Vector2f pos, sf::Color color, ObjectType t, sf::Vector2i gridpos);

    virtual void draw(sf::RenderWindow &window);

    sf::FloatRect getBounds() const;
    ObjectType getType() const;
    sf::Vector2f getPos() const;
    sf::Vector2i getGridPosition() const;

    void setGridPosition(sf::Vector2f worldPos, float tileSize);
    void setPos(sf::Vector2f &pos);
    void setFillColor(sf::Color color);
};