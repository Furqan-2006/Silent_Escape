#pragma once

#include <string>
#include <vector>
#include "player.hpp"
#include "guard.hpp"

struct GuardMetadata
{
    sf::Vector2f position;
    sf::Vector2f direction;
    std::vector<sf::Vector2f> patrolPath;

    void addPoint(const sf::Vector2f &point)
{
 patrolPath.push_back(point);
}
};

struct LevelMetadata
{
    sf::Vector2f playerPos;
    std::vector<GuardMetadata> guards;
};

LevelMetadata loadMetadata(const std::string &jsonPath, float tileSize);
