#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>

class Guard
{
private:
    sf::CircleShape circle;

public:
    Guard();
    void chase();
    void capture();
    void alert();
    void patrol();

    void draw(sf::RenderWindow &window);
};