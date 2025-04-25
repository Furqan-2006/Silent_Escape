#include "pathfinder.hpp"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>
#include <SFML/System.hpp>

PathFinder::PathFinder(int rows, int cols, float tileSize, const std::vector<std::vector<int>> &mapData)
    : rows(rows), cols(cols), tileSize(tileSize), grid(mapData) {}

bool PathFinder::isWalkable(int row, int col)
{
    return (row >= 0 && row < rows && col >= 0 && col < cols && grid[row][col] == 0);
}
struct CompareFCost
{
    bool operator()(const std::shared_ptr<TileNode> &a, const std::shared_ptr<TileNode> &b)
    {
        return a->fCost() > b->fCost();
    }
};

// Manhattan heuristic
float heuristic(const sf::Vector2i &a, const sf::Vector2i &b)
{
    int dx = std::abs(a.x - b.x);
    int dy = std::abs(a.y - b.y);
    return static_cast<float>(dx + dy);
}

std::vector<sf::Vector2i> PathFinder::getNeighbours(const sf::Vector2i &pos)
{
    std::vector<sf::Vector2i> neighbours;
    const std::vector<sf::Vector2i> directions = {
        {0, -1}, {0, 1}, {-1, 0}, {1, 0}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

    for (const auto &dir : directions)
    {
        sf::Vector2i neighbour = pos + dir;
        if (isWalkable(neighbour.y, neighbour.x))
        {
            if (std::abs(dir.x) == 1 && std::abs(dir.y) == 1)
            {
                if (!isDiagonalMoveValid(pos, neighbour))
                    continue;
            }
            neighbours.push_back(neighbour);
        }
    }

    return neighbours;
}

bool PathFinder::isDiagonalMoveValid(const sf::Vector2i &from, const sf::Vector2i &to)
{
    int dx = to.x - from.x;
    int dy = to.y - from.y;

    if (dx != 0 && dy != 0)
    {
        return isWalkable(from.y, from.x + dx) && isWalkable(from.y + dy, from.x);
    }
    return true;
}

std::vector<sf::Vector2f> PathFinder::findPath(const sf::Vector2f &startPos, const sf::Vector2f &endPos)
{
    std::cout << "Finding path from (" << startPos.x << ", " << startPos.y << ") to (" << endPos.x << ", " << endPos.y << ")\n";

    sf::Vector2i start(static_cast<int>(startPos.x / tileSize), static_cast<int>(startPos.y / tileSize));
    sf::Vector2i end(static_cast<int>(endPos.x / tileSize), static_cast<int>(endPos.y / tileSize));

    if (!isWalkable(start.y, start.x) || !isWalkable(end.y, end.x))
        return {};

    std::priority_queue<std::shared_ptr<TileNode>, std::vector<std::shared_ptr<TileNode>>, CompareFCost> openSet;
    std::unordered_map<int, std::shared_ptr<TileNode>> allNodes;

    auto toKey = [&](const sf::Vector2i &p)
    { return p.y * cols + p.x; };

    auto startNode = std::make_shared<TileNode>(
        TileNode{start, 0.0f, heuristic(start, end), nullptr});
    openSet.push(startNode);
    allNodes[toKey(start)] = startNode;

    std::unordered_set<int> closedSet;

    while (!openSet.empty())
    {
        auto current = openSet.top();
        openSet.pop();

        int currentKey = toKey(current->position);
        if (closedSet.count(currentKey))
            continue;

        closedSet.insert(currentKey);

        std::cout << "Evaluating node: (" << current->position.x << ", " << current->position.y << ")\n";

        if (current->position == end)
        {
            std::vector<sf::Vector2f> path;
            auto node = current;
            std::cout << "Evaluating node: (" << current->position.x << ", " << current->position.y << ") | gCost: " << current->gCost << " | fCost: " << current->fCost() << "\n";

            while (node)
            {
                path.push_back(sf::Vector2f(node->position.x * tileSize, node->position.y * tileSize));
                node = node->parent;
            }
            std::reverse(path.begin(), path.end());
            std::cout << "Path found:\n";
            for (const auto &p : path)
            {
                std::cout << "  -> (" << p.x << ", " << p.y << ")\n";
            }
            return smoothPath(path);
            std::cout << "Path found:\n";
        }

        for (const auto &neighborPos : getNeighbours(current->position))
        {
            if (closedSet.count(toKey(neighborPos)))
                continue;
            float cost = (neighborPos.x == current->position.x || neighborPos.y == current->position.y) ? 1.0f : std::sqrt(2.0f);
            float newGCost = current->gCost + cost;

            int key = toKey(neighborPos);
            auto it = allNodes.find(key);

            if (it == allNodes.end())
            {
                auto neighbor = std::make_shared<TileNode>(TileNode{
                    neighborPos, newGCost, heuristic(neighborPos, end), current});
                openSet.push(neighbor);
                allNodes[key] = neighbor;
                std::cout << " -> Neighbor: (" << neighborPos.x << ", " << neighborPos.y << ") | gCost: " << newGCost << "\n";
            }
            else if (newGCost < it->second->gCost)
            {
                // Update the existing node with better path
                it->second->gCost = newGCost;
                it->second->parent = current;

                // Push updated node again - older one will be ignored due to closedSet
                openSet.push(it->second);
                std::cout << " -> Updated Neighbor: (" << neighborPos.x << ", " << neighborPos.y << ") | gCost: " << newGCost << "\n";
            }
        }
    }
    std::cout << "No path found.\n";

    return {};
}

bool PathFinder::hasLineOfSight(const sf::Vector2i &start, const sf::Vector2i &end)
{
    int x0 = start.x, y0 = start.y;
    int x1 = end.x, y1 = end.y;

    int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (x0 != x1 || y0 != y1)
    {
        if (!isWalkable(y0, x0))
            return false;

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
}

std::vector<sf::Vector2f> PathFinder::smoothPath(const std::vector<sf::Vector2f> &path)
{
    if (path.size() < 3)
        return path;

    std::vector<sf::Vector2f> result;
    size_t i = 0;

    while (i < path.size())
    {
        size_t j = i + 1;
        sf::Vector2i from(path[i].x / tileSize, path[i].y / tileSize);

        for (; j < path.size(); ++j)
        {
            sf::Vector2i to(path[j].x / tileSize, path[j].y / tileSize);
            if (!hasLineOfSight(from, to))
                break;
        }

        result.push_back(path[i]);
        i = j - 1;
    }

    if (result.back() != path.back())
        result.push_back(path.back());

    return result;
}
