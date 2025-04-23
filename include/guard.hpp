#pragma once

#include <iostream>
#include <cmath>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include "gameObj.hpp"
#include "player.hpp"
#include "gameStateEnum.hpp"
#include "pathfinder.hpp"

#define PI 3.14159265358979323846f

enum class GuardState
{
    Patrolling,
    Alerted,
    Chasing,
    Searching,
    Idle
};
enum SearchPhase
{
    LookAround,
    Wander,
    ReturnToPatrol
};

class Guard
{
private:
    GuardState state;
    SearchPhase currentPhase = SearchPhase::LookAround;

    sf::Vector2f initialPosition;

    sf::CircleShape circle;
    sf::CircleShape lastSeenMarker;
    sf::Vector2f velocity;
    sf::Vector2f facingDir;
    sf::Clock alertClock;
    sf::Clock searchClock;
    sf::Clock wanderPauseClock;

    const float fieldOfView = 60.f;
    const float alertDuration = 0.5f;
    const float searchDuration = 5.f;
    float viewDistance = 100.f;
    float moveSpeed = 1.0f;
    float pauseDurationAtTarget = 1.f;

    bool alertClockStarted = false;
    bool returnedToInitialPos = false;
    bool sightCone = true;
    bool searchClockStarted = false;
    bool isPausingAtTarget = false;
    bool generatedInitialWanderPath = false;

    int currentWanderIndex = 0;
    int currentWanderPathIndex = 0;
    int currentWanderTarget = 0;

    sf::Color sightColor = sf::Color(255, 255, 0, 100);
    sf::Vector2f searchDirection;
    sf::Vector2f lastKnownPlayerPosition;
    std::vector<sf::Vector2f> wanderPath;
    std::vector<sf::Vector2f> wanderTargets;

public:
    Guard(const sf::Vector2f &startPos);
    Guard();

    void patrol(const std::vector<GameObject> &obstacles);
    void alert();
    void chase(const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles, PathFinder &pathfinder);
    void search(const std::vector<GameObject> &obstacles, PathFinder &pathfinder, const sf::Vector2f &lastPlayerPos, float &tileSize);
    void capture(GameState &gameState);

    bool canSeePlayer(const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles);
    void drawSightCone(sf::RenderWindow &window);

    // bool canHearPlayer();

    void update(Player &player, const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles, GameState &gameState, PathFinder &pathfinder, sf::RenderWindow &window, float &tileSize);
    bool checkCollision(const sf::FloatRect &otherBounds) const;
    void draw(sf::RenderWindow &window);

    void setPosition(const sf::Vector2f &position);
    void setVelocity(const sf::Vector2f &dir);
    sf::FloatRect getBounds() const;
};