#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>

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

public:
    bool isWalkable(int row, int col);
    PathFinder();
    PathFinder(int rows, int cols, float tileSize, const std::vector<std::vector<int>> &mapData);
    std::vector<sf::Vector2f> findPath(const sf::Vector2f &startPos, const sf::Vector2f &endPos);
};
