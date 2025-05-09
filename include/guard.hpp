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
#define TILE_SIZE 4
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
    sf::Vector2i gridPosition;
    std::vector<sf::Vector2f> patrolPath;
    std::vector<sf::Vector2f> returnPath;

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
    float moveSpeed = 60.0f;
    float pauseDurationAtTarget = 1.f;
    float pathTimer = 0.f;
    float repathInterval = 0.5f;

    bool generatedInitialPatrolPath = false;
    bool alertClockStarted = false;
    bool returnedToInitialPos = false;
    bool sightCone = true;
    bool searchClockStarted = false;
    bool isPausingAtTarget = false;
    bool generatedInitialWanderPath = false;
    bool hasCurrentWanderPath = false;
    bool isReturningToPatrol = false;

    int currentPatrolIndex = 0;
    int currentPatrolPathIndex = 0;
    int currentWanderIndex = 0;
    int currentWanderPathIndex = 0;
    int currentWanderTarget = 0;
    int returnPathIndex = 0;

    sf::Color sightColor = sf::Color(255, 255, 0, 100);
    sf::Vector2f searchDirection;
    sf::Vector2f lastKnownPlayerPosition;
    std::vector<sf::Vector2f> patrolPathToTarget;
    std::vector<sf::Vector2f> wanderPath;
    std::vector<sf::Vector2f> wanderTargets;
    std::vector<sf::Vector2f> currentPath;

    float tileSize = 40.f;

public:
    Guard(const sf::Vector2f &startPos);
    Guard();

    void patrol(const std::vector<GameObject> &obstacles, PathFinder &pathfinder, float &deltaTime);
    void alert();
    void chase(const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles, PathFinder &pathfinder, float &deltaTime);
    void search(const std::vector<GameObject> &obstacles, PathFinder &pathfinder, const sf::Vector2f &lastPlayerPos, float &tileSize, float &deltaTime);
    void capture(GameState &gameState, const sf::Vector2f &playerPos, sf::Clock &gameOverClock);

    bool canSeePlayer(const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles);
    void drawSightCone(sf::RenderWindow &window);
    // void drawPath(sf::RenderWindow &window);

    // bool canHearPlayer();

    void update(Player &player, const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles, GameState &gameState, PathFinder &pathfinder, sf::RenderWindow &window, float &tileSize, float &deltaTime, sf::Clock &gameOverClock);

    bool checkCollision(const sf::FloatRect &otherBounds) const;
    void draw(sf::RenderWindow &window);

    sf::Vector2i getGridPosition() const;

    void setPos(const sf::Vector2f &position);
    void setGridPos(const sf::Vector2i &position);
    void setVelocity(const sf::Vector2f &dir);
    void setPatrolPath(const std::vector<sf::Vector2f> &path);
    void resetState();

    sf::FloatRect getBounds() const;
};