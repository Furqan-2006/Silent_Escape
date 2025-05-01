#pragma once

#include <SFML/System.hpp>
#include <vector>

class PathFinder
{
private:
    std::vector<std::vector<int>> grid;

    struct Node;
    struct NodeComparator;
    struct Vector2fHash;
    struct Vector2fEq;
    int width, height;

    float tileSize;
    float step;

public:
    PathFinder(float stepSize, int worldWidth, int worldHeight, const std::vector<std::vector<int>> &mapData, float tileSize = 40.f);

    // Find a path from start to goal (both world pixel positions).
    // Returns a vector of positions from start to goal (inclusive).
    std::vector<sf::Vector2f> findPath(sf::Vector2f start, sf::Vector2f goal);

    // Customize your own walkability check based on your game collision.
    sf::Vector2f roundToTileCenter(const sf::Vector2f &pixelPos) const;
    // void debugGrid(sf::Vector2f start, sf::Vector2f goal) const;
    bool isWalkable(const sf::Vector2f &point) const;
};
