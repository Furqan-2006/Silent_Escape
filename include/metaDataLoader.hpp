#pragma once

#include <string>
#include <vector>
#include "player.hpp"
#include "guard.hpp"

struct GuardMetadata
{
    sf::Vector2f position;
    sf::Vector2f direction;
};

struct LevelMetadata
{
    sf::Vector2f playerPos;
    std::vector<GuardMetadata> guards; 
};

LevelMetadata loadMetadata(const std::string& jsonPath, float tileSize);