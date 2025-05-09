#pragma once

#include <SFML/Graphics.hpp>

class Distraction
{
    sf::CircleShape stone;
    float duration;
    sf::Clock clock;

public:
    Distraction(const sf::Vector2f &pos, float radius = 10.f, float dur = 3.f);
    bool isExpired() const;
    void draw(sf::RenderWindow &win);
    sf::Vector2f getPos() const;
    float getRadius() const;
};