#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>

class GameObject
{
private:
    sf::CircleShape shape;

public:
    GameObject(float rad, int points, sf::Vector2f pos, sf::Color color);
    virtual void draw(sf::RenderWindow &window);
    sf::FloatRect getBounds() const;
    
};