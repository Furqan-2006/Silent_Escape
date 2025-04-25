#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <vector>

struct TileNode
{
    sf::Vector2i position;
    float gCost, hCost;
    std::shared_ptr<TileNode> parent;

    float fCost() const { return gCost + hCost; }

    bool operator==(const TileNode &other) const
    {
        return position == other.position;
    }
};

class PathFinder
{
private:
    std::vector<std::vector<int>> grid;
    int rows, cols;
    float tileSize;

    std::vector<sf::Vector2i> getNeighbours(const sf::Vector2i &pos);
    bool hasLineOfSight(const sf::Vector2i &start, const sf::Vector2i &end);
    std::vector<sf::Vector2f> smoothPath(const std::vector<sf::Vector2f> &path);

public:
    bool isWalkable(int row, int col);
    PathFinder() {}
    PathFinder(int rows, int cols, float tileSize, const std::vector<std::vector<int>> &mapData);
    std::vector<sf::Vector2f> findPath(const sf::Vector2f &startPos, const sf::Vector2f &endPos);
    bool isDiagonalMoveValid(const sf::Vector2i &from, const sf::Vector2i &to);
};
