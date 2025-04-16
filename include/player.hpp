#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>
#include "gameObj.hpp"

class Player
{
protected:
    sf::RectangleShape rect;
    float speed;

public:
    Player(float sp = 5);
    void moveUp(const std::vector<GameObject> &obstacles);
    void moveDown(const std::vector<GameObject> &obstacles);
    void moveRight(const std::vector<GameObject> &obstacles);
    void moveLeft(const std::vector<GameObject> &obstacles);
    void disguise();
    void hide();
    void hack();

    void draw(sf::RenderWindow &window);
    bool checkCollision(const sf::FloatRect &otherBounds) const;
};
