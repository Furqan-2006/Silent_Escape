#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>
#include "utils.hpp"
#include <memory>

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
    std::unique_ptr<sf::Sprite> sprite;

    ObjectType type;
    sf::Vector2i gridPos;
    sf::FloatRect collisionBox;

public:
    GameObject(const sf::Texture &tex, sf::Vector2f pos, ObjectType t, sf::Vector2i gridpos, float tileSize);

    virtual void draw(sf::RenderWindow &window);

    sf::FloatRect getBounds() const;
    ObjectType getType() const;
    sf::Vector2f getPos() const;
    sf::Vector2i getGridPosition() const;
    sf::FloatRect getCollisionBox() const;

    void setPos(sf::Vector2f &pos);
};