#ifndef PATHFINDER_HPP
#define PATHFINDER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include "gameObj.hpp"

class PathFinder
{
private:
    struct Node;
    struct NodeComparator;
    struct Vector2fHash;
    struct Vector2fEq;

    std::vector<std::vector<bool>> grid;
    const std::vector<std::vector<int>>& mapData; // Const reference to mapData
    float tileSize;
    float step;
    int width, height;

    std::vector<sf::Vector2f> smoothPath(const std::vector<sf::Vector2f>& path) const;
    bool isLineWalkable(const sf::Vector2f& start, const sf::Vector2f& end) const;

public:
    PathFinder(float stepSize, int worldWidth, int worldHeight, const std::vector<std::vector<int>> &mapData, float tileSize = 40.f);
    sf::Vector2f roundToTileCenter(const sf::Vector2f &pixelPos) const;
    bool isWalkable(const sf::Vector2f &point) const;
    std::vector<sf::Vector2f> findPath(sf::Vector2f start, sf::Vector2f goal);
    void updateObstacles(const std::vector<GameObject>& obstacles);
};

#endif