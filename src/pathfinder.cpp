#include "PathFinder.hpp"

#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>

#ifdef DEBUG
#include <iostream>
#define LOG(x) std::cout << x << std::endl
#else
#define LOG(x)
#endif

struct CompareFCost
{
    bool operator()(const std::shared_ptr<TileNode> &a, const std::shared_ptr<TileNode> &b)
    {
        return a->fCost() > b->fCost();
    }
};

float heuristic(const sf::Vector2i &a, const sf::Vector2i &b)
{
    return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

PathFinder::PathFinder(int rows, int cols, float tileSize, const std::vector<std::vector<int>> &mapData)
    : rows(rows), cols(cols), tileSize(tileSize), grid(mapData) {}

bool PathFinder::isWalkable(int row, int col) const
{
    return (row >= 0 && row < rows && col >= 0 && col < cols && grid[row][col] == 0);
}

std::vector<sf::Vector2i> PathFinder::getNeighbours(const sf::Vector2i &pos) const
{
    static const std::vector<sf::Vector2i> directions = {
        {0, -1}, {0, 1}, {-1, 0}, {1, 0}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

    std::vector<sf::Vector2i> neighbours;
    for (const auto &dir : directions)
    {
        sf::Vector2i neighbour = pos + dir;
        if (!isWalkable(neighbour.y, neighbour.x))
            continue;

        // Diagonal movement check
        if (std::abs(dir.x) == 1 && std::abs(dir.y) == 1)
        {
            if (!isWalkable(pos.y, pos.x + dir.x) || !isWalkable(pos.y + dir.y, pos.x))
                continue;
        }

        neighbours.push_back(neighbour);
    }
    return neighbours;
}

std::vector<sf::Vector2f> PathFinder::findPath(const sf::Vector2f &startPos, const sf::Vector2f &endPos)
{
    sf::Vector2i start(static_cast<int>(startPos.x / tileSize), static_cast<int>(startPos.y / tileSize));
    sf::Vector2i end(static_cast<int>(endPos.x / tileSize), static_cast<int>(endPos.y / tileSize));

    if (!isWalkable(start.y, start.x) || !isWalkable(end.y, end.x))
        return {};

    auto toKey = [&](const sf::Vector2i &p)
    { return p.y * cols + p.x; };

    std::priority_queue<std::shared_ptr<TileNode>, std::vector<std::shared_ptr<TileNode>>, CompareFCost> openSet;
    std::unordered_map<int, std::shared_ptr<TileNode>> allNodes;
    std::unordered_set<int> closedSet;

    auto startNode = std::make_shared<TileNode>(TileNode{start, 0.0f, heuristic(start, end), nullptr});
    openSet.push(startNode);
    allNodes[toKey(start)] = startNode;

    while (!openSet.empty())
    {
        auto current = openSet.top();
        openSet.pop();

        int currentKey = toKey(current->position);
        if (closedSet.count(currentKey))
            continue;

        closedSet.insert(currentKey);

        if (current->position == end)
        {
            std::vector<sf::Vector2f> path;
            auto node = current;
            while (node)
            {
                path.push_back(sf::Vector2f(node->position.x * tileSize + tileSize / 2, node->position.y * tileSize + tileSize / 2));
                node = node->parent;
            }
            std::reverse(path.begin(), path.end());

            if (!path.empty() && std::hypot(endPos.x - path.back().x, endPos.y - path.back().y) < tileSize)
                path.push_back(endPos);

            return smoothPath(path);
        }

        for (const auto &neighborPos : getNeighbours(current->position))
        {
            int neighborKey = toKey(neighborPos);
            if (closedSet.count(neighborKey))
                continue;

            float cost = (neighborPos.x == current->position.x || neighborPos.y == current->position.y) ? 1.0f : std::sqrt(2.0f);
            float tentativeGCost = current->gCost + cost;

            auto it = allNodes.find(neighborKey);
            if (it == allNodes.end() || tentativeGCost < it->second->gCost)
            {
                auto neighborNode = std::make_shared<TileNode>(TileNode{
                    neighborPos, tentativeGCost, heuristic(neighborPos, end), current});

                openSet.push(neighborNode);
                allNodes[neighborKey] = neighborNode;
            }
        }
    }

    return {};
}

std::vector<sf::Vector2f> PathFinder::smoothPath(const std::vector<sf::Vector2f> &path) const
{
    if (path.size() < 2)
        return path;

    std::vector<sf::Vector2f> result;
    size_t i = 0;

    auto hasLineOfSight = [&](const sf::Vector2i &start, const sf::Vector2i &end) -> bool
    {
        int x0 = start.x, y0 = start.y;
        int x1 = end.x, y1 = end.y;
        int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;

        while (true)
        {
            if (!isWalkable(y0, x0))
                return false;
            if (x0 == x1 && y0 == y1)
                break;

            int e2 = 2 * err;
            if (e2 > -dy)
            {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx)
            {
                err += dx;
                y0 += sy;
            }
        }
        return true;
    };

    while (i < path.size())
    {
        size_t j = i + 1;
        sf::Vector2i from(static_cast<int>(path[i].x / tileSize), static_cast<int>(path[i].y / tileSize));

        while (j < path.size())
        {
            sf::Vector2i to(static_cast<int>(path[j].x / tileSize), static_cast<int>(path[j].y / tileSize));
            if (!hasLineOfSight(from, to))
                break;
            ++j;
        }

        result.push_back(path[i]);
        i = (j > i + 1) ? (j - 1) : (i + 1);
    }

    if (!result.empty() && result.back() != path.back())
        result.push_back(path.back());

    return result;
}

