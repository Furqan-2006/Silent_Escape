#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>
#include <algorithm>
#include "gameObj.hpp"
#include "pathfinder.hpp"
#include <unordered_map>
#include "distraction.hpp"
#include "utils.hpp"

class Player
{
protected:
    sf::RectangleShape rect;

    sf::Vector2i gridPosition;

    bool disguised;
    bool hidden;

    std::unordered_map<std::string, sf::Clock> actionClocks;
    std::unordered_map<std::string, float> actionCooldowns;
    std::vector<Distraction> distractions;

    float speed;

public:
    Player(float sp = 1.f);

    bool canMove(sf::Vector2f offset, const std::vector<GameObject> &obstacles);
    void moveUp(float deltaTime, const std::vector<GameObject> &obstacles);
    void moveDown(float deltaTime, const std::vector<GameObject> &obstacles);
    void moveRight(float deltaTime, const std::vector<GameObject> &obstacles);
    void moveLeft(float deltaTime, const std::vector<GameObject> &obstacles);

    void draw(sf::RenderWindow &window);
    void update();

    void setDisguised(bool Value);
    void setHidden(bool Value);
    void setActionCooldown(std::string action, float duration);
    void setPos(sf::Vector2f &pos);
    void setGridPos(sf::Vector2i &pos);

    bool isDisguised() const;
    bool isHidden() const;
    // bool hasItem() const;
    bool isOnCooldown(std::string action) const;
    bool checkCollision(const sf::FloatRect &otherBounds) const;

    sf::FloatRect getBounds() const;
    sf::Vector2f getPos() const;
    sf::Vector2i getGridPosition() const;

    void disguise();
    void hide();
    void hack();
    void distract(); // throw distraction
    void cleanupDistractions();

    const std::vector<Distraction> &getDistractions() const;

    void drawTileHighlight(sf::RenderWindow &window, int tileWidth, int tileHeight);
};