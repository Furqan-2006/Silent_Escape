#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>
#include "gameObj.hpp"

class Player
{
protected:
    sf::RectangleShape rect;
    float speed;
    bool disguised;
    bool hidden;

public:
    Player(float sp = 0.2f);
    void moveUp(const std::vector<GameObject> &obstacles);
    void moveDown(const std::vector<GameObject> &obstacles);
    void moveRight(const std::vector<GameObject> &obstacles);
    void moveLeft(const std::vector<GameObject> &obstacles);

    void setDisguised(bool Value);
    bool isDisguised() const;
    void setHidden(bool Value);
    bool isHidden() const;

    void disguise();
    void hide();
    void hack();

    void draw(sf::RenderWindow &window);
    bool checkCollision(const sf::FloatRect &otherBounds) const;
    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const;
    void setPosition(sf::Vector2f pos);
};