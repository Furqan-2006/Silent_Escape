#pragma once

#include <iostream>
#include <cmath>
#include <SFML/Graphics.hpp>
#include "gameObj.hpp"
#include "player.hpp"
#include "gameStateEnum.hpp"

#define PI 3.14159265358979323846f

enum class GuardState
{
    Patrolling,
    Alerted,
    Chasing,
    Searching,
    Idle
};
class Guard
{
private:
    GuardState state;
    sf::CircleShape circle;
    sf::Vector2f velocity;
    sf::Vector2f facingDir;
    const float fieldOfView = 60.f;
    const float viewDistance = 150.f;
    bool sightCone = true;
    sf::Color sightColor = sf::Color(255, 255, 0, 100);

public:
    Guard();
    void chase(const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles);
    void capture(GameState &gameState);
    void alert();
    void patrol(const std::vector<GameObject> &obstacles);

    bool canSeePlayer(const sf::Vector2f &playerPos);
    void drawSightCone(sf::RenderWindow &window);

    void update(Player &player, const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles, GameState &gameState);
    bool checkCollision(const sf::FloatRect &otherBounds) const;
    void draw(sf::RenderWindow &window);
    sf::FloatRect getBounds() const;
};