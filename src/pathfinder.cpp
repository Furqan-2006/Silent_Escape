#include "pathfinder.hpp"
#include <algorithm>
#include <queue>
#include <cmath>

PathFinder::PathFinder(int rows, int cols, float tileSize, const std::vector<std::vector<int>> &mapData)
    : rows(rows), cols(cols), tileSize(tileSize), grid(mapData) {}

bool PathFinder::isWalkable(int row, int col)
{
    return (row >= 0 && row < rows && col >= 0 && col < cols && grid[row][col] == 0);
}

std::vector<sf::Vector2i> PathFinder::getNeighbours(const sf::Vector2i &pos)
{

    std::vector<sf::Vector2i> neighbours;
    const std::vector<sf::Vector2i> directions = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};

    for (const auto &dir : directions)
    {
        sf::Vector2i neighbour = pos + dir;
        if (isWalkable(neighbour.y, neighbour.x))
        {
            neighbours.push_back(neighbour);
        }
    }
    return neighbours;
}

float heuristic(const sf::Vector2i &a, const sf::Vector2i &b)
{
    return std::abs(a.x - b.x) + std::abs(a.y - b.y); // Manhattan distance
}

std::vector<sf::Vector2f> PathFinder::findPath(const sf::Vector2f &startPos, const sf::Vector2f &endPos)
{
    sf::Vector2i start(static_cast<int>(startPos.x / tileSize), static_cast<int>(startPos.y / tileSize));
    sf::Vector2i end(static_cast<int>(endPos.x / tileSize), static_cast<int>(endPos.y / tileSize));

    std::vector<TileNode> openList;
    std::vector<TileNode> closedList;

    TileNode startNode{start, 0, heuristic(start, end), nullptr};
    openList.push_back(startNode);

    while (!openList.empty())
    {
        auto currentIt = std::min_element(openList.begin(), openList.end(),
                                          [](const TileNode &a, const TileNode &b)
                                          { return a.fCost() < b.fCost(); });
        TileNode current = *currentIt;
        openList.erase(currentIt);

        if (current.position == end)
        {
            std::vector<sf::Vector2f> path;
            std::shared_ptr<TileNode> node = std::make_shared<TileNode>(current);
            while (node)
            {
                path.push_back(sf::Vector2f(node->position.x * tileSize, node->position.y * tileSize));
                node = node->parent;
            }

            return path;
        }

        closedList.push_back(current);

        for (const auto &neighborPos : getNeighbours(current.position))
        {
            if (std::any_of(closedList.begin(), closedList.end(),
                            [&](const TileNode &n)
                            { return n.position == neighborPos; }))
                continue;

            float gCost = current.gCost + 1;
            float hCost = heuristic(neighborPos, end);
            std::shared_ptr<TileNode> currentPtr = std::make_shared<TileNode>(current);
            TileNode neighbor{neighborPos, gCost, hCost, currentPtr};

            auto it = std::find_if(openList.begin(), openList.end(),
                                   [&](const TileNode &n)
                                   { return n.position == neighborPos; });

            if (it == openList.end() || gCost < it->gCost)
            {
                openList.push_back(neighbor);
            }
        }
    }

    return {};
}
