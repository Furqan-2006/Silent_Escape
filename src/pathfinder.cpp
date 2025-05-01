#include "PathFinder.hpp"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>

#ifdef DEBUG_PATHFINDER
#define PATHFINDER_LOG(x) std::cout << "[PathFinder] " << x << "\n"
#else
#define PATHFINDER_LOG(x)
#endif

struct PathFinder::Node
{
    sf::Vector2f pos, parentPos;
    float gCost, hCost;
    Node(sf::Vector2f position = {}, sf::Vector2f parent = {}, float g = 0.f, float h = 0.f)
        : pos(position), parentPos(parent), gCost(g), hCost(h) {}
    float fCost() const { return gCost + hCost; }
};

struct PathFinder::NodeComparator
{
    bool operator()(const Node &a, const Node &b) const
    {
        return a.fCost() > b.fCost();
    }
};

struct PathFinder::Vector2fHash
{
    std::size_t operator()(const sf::Vector2f &v) const noexcept
    {
        return std::hash<int>()(static_cast<int>(v.x)) ^ (std::hash<int>()(static_cast<int>(v.y)) << 1);
    }
};

struct PathFinder::Vector2fEq
{
    bool operator()(const sf::Vector2f &a, const sf::Vector2f &b) const noexcept
    {
        return a.x == b.x && a.y == b.y;
    }
};

PathFinder::PathFinder(float stepSize, int worldWidth, int worldHeight, const std::vector<std::vector<int>> &mapData, float tileSize)
    : step(stepSize), width(worldWidth), height(worldHeight), tileSize(tileSize), mapData(mapData)
{
    grid.resize(width, std::vector<bool>(height, true));
    for (int y = 0; y < height && y < mapData.size(); ++y)
        for (int x = 0; x < width && x < mapData[y].size(); ++x)
            grid[x][y] = mapData[y][x] == 0; // 0 = walkable, 1 = blocked
    PATHFINDER_LOG("Initialized with stepSize=" << stepSize << ", worldSize=(" << worldWidth << "x" << worldHeight << "), tileSize=" << tileSize);
}

void PathFinder::updateObstacles(const std::vector<GameObject> &obstacles)
{
    for (int y = 0; y < height && y < mapData.size(); ++y)
        for (int x = 0; x < width && x < mapData[y].size(); ++x)
            grid[x][y] = mapData[y][x] == 0;

    for (const auto &obj : obstacles)
    {
        if (obj.getType() == ObjectType::Wall || obj.getType() == ObjectType::Box || obj.getType() == ObjectType::Door)
        {
            sf::FloatRect bounds = obj.getBounds();
            if (bounds.size.x <= 0 || bounds.size.y <= 0)
                continue;

            // Adjust bounds for CircleShape to reduce overestimation
            float radius = bounds.size.x / 2.f;
            float shrinkFactor = 0.7f; // Adjust based on testing
            bounds.position.x += radius * (1 - shrinkFactor);
            bounds.position.y += radius * (1 - shrinkFactor);
            bounds.size.x *= shrinkFactor;
            bounds.size.y *= shrinkFactor;

            int minX = std::max(0, static_cast<int>(bounds.position.x / tileSize));
            int maxX = std::min(width - 1, static_cast<int>((bounds.position.x + bounds.size.x) / tileSize));
            int minY = std::max(0, static_cast<int>(bounds.position.y / tileSize));
            int maxY = std::min(height - 1, static_cast<int>((bounds.position.y + bounds.size.y) / tileSize));

            for (int x = minX; x <= maxX; ++x)
                for (int y = minY; y <= maxY; ++y)
                    grid[x][y] = false;
        }
    }
}
sf::Vector2f PathFinder::roundToTileCenter(const sf::Vector2f &pixelPos) const
{
    int tileX = static_cast<int>(pixelPos.x / tileSize);
    int tileY = static_cast<int>(pixelPos.y / tileSize);
    sf::Vector2f rounded(tileX * tileSize + tileSize / 2.f, tileY * tileSize + tileSize / 2.f);
    PATHFINDER_LOG("roundToTileCenter: (" << pixelPos.x << "," << pixelPos.y << ") -> (" << rounded.x << "," << rounded.y << ")");
    return rounded;
}

bool PathFinder::isWalkable(const sf::Vector2f &point) const
{
    int tileX = static_cast<int>(point.x / tileSize);
    int tileY = static_cast<int>(point.y / tileSize);
    bool walkable = tileX >= 0 && tileX < width && tileY >= 0 && tileY < height && grid[tileX][tileY];
    PATHFINDER_LOG("isWalkable: (" << point.x << "," << point.y << ") -> tile (" << tileX << "," << tileY << ") -> " << (walkable ? "WALKABLE" : "BLOCKED"));
    return walkable;
}

bool PathFinder::isLineWalkable(const sf::Vector2f &start, const sf::Vector2f &end) const
{
    sf::Vector2f dir = end - start;
    float distance = hypot(dir.x, dir.y);
    if (distance == 0)
        return true;
    dir /= distance;

    for (float t = 0; t <= distance; t += tileSize / 4.f)
    {
        sf::Vector2f point = start + dir * t;
        if (!isWalkable(point))
            return false;
    }
    return true;
}

std::vector<sf::Vector2f> PathFinder::smoothPath(const std::vector<sf::Vector2f> &path) const
{
    if (path.size() < 2)
        return path;

    std::vector<sf::Vector2f> smoothed;
    smoothed.push_back(path[0]);

    for (size_t i = 1; i < path.size() - 1; ++i)
    {
        sf::Vector2f start = smoothed.back();
        sf::Vector2f end = path[i + 1];
        if (isLineWalkable(start, end))
            continue;
        smoothed.push_back(path[i]);
    }
    smoothed.push_back(path.back());

    PATHFINDER_LOG("Smoothed path from " << path.size() << " to " << smoothed.size() << " points");
    return smoothed;
}

std::vector<sf::Vector2f> PathFinder::findPath(sf::Vector2f start, sf::Vector2f goal)
{
    PATHFINDER_LOG("START Pathfinding from (" << start.x << ", " << start.y << ") to (" << goal.x << ", " << goal.y << ")");

    start = roundToTileCenter(start);
    goal = roundToTileCenter(goal);

    if (!isWalkable(start))
    {
        PATHFINDER_LOG("ERROR: Start position is not walkable!");
        return {};
    }

    if (!isWalkable(goal))
    {
        const std::array<sf::Vector2f, 4> directions = {
            sf::Vector2f(tileSize, 0),
            sf::Vector2f(-tileSize, 0),
            sf::Vector2f(0, tileSize),
            sf::Vector2f(0, -tileSize)};
        for (const auto &dir : directions)
        {
            sf::Vector2f newGoal = goal + dir;
            if (isWalkable(newGoal))
            {
                goal = newGoal;
                PATHFINDER_LOG("Adjusted goal to walkable point: (" << goal.x << "," << goal.y << ")");
                break;
            }
        }
        if (!isWalkable(goal))
        {
            PATHFINDER_LOG("ERROR: Goal position is not walkable!");
            return {};
        }
    }

    if (hypot(goal.x - start.x, goal.y - start.y) < tileSize * 2 && isLineWalkable(start, goal))
    {
        PATHFINDER_LOG("Direct path used for nearby target");
        return {start, goal};
    }

    std::vector<sf::Vector2f> directions = {
        {step, 0}, {-step, 0}, {0, step}, {0, -step}, {step, step}, {step, -step}, {-step, step}, {-step, -step}};

    std::unordered_map<sf::Vector2f, Node, Vector2fHash, Vector2fEq> nodes;
    std::unordered_map<sf::Vector2f, float, Vector2fHash, Vector2fEq> openCosts;
    std::priority_queue<Node, std::vector<Node>, NodeComparator> openHeap;
    std::unordered_set<sf::Vector2f, Vector2fHash, Vector2fEq> closedSet;

    nodes[start] = Node(start, start, 0.f, hypot(goal.x - start.x, goal.y - start.y));
    openHeap.push(nodes[start]);
    openCosts[start] = nodes[start].fCost();

    int iterations = 0;
    int maxIterations = 10000;

    while (!openHeap.empty() && iterations++ < maxIterations)
    {
        Node current = openHeap.top();
        openHeap.pop();
        openCosts.erase(current.pos);

        if (closedSet.count(current.pos))
            continue;

        if (current.pos == goal)
        {
            std::vector<sf::Vector2f> path;
            for (sf::Vector2f pos = goal; pos != start; pos = nodes[pos].parentPos)
                path.push_back(pos);
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            path = smoothPath(path);
            PATHFINDER_LOG("COMPLETE. Path length: " << path.size() << ", iterations: " << iterations);
            return path;
        }

        closedSet.insert(current.pos);

        for (const auto &dir : directions)
        {
            sf::Vector2f neighborPos = current.pos + dir;
            if (!isWalkable(neighborPos) || closedSet.count(neighborPos))
                continue;

            float moveCost = hypot(dir.x, dir.y);
            float tentativeG = current.gCost + moveCost;

            auto it = nodes.find(neighborPos);
            if (it == nodes.end())
            {
                nodes[neighborPos] = Node(neighborPos, current.pos, tentativeG, hypot(goal.x - neighborPos.x, goal.y - neighborPos.y));
                openHeap.push(nodes[neighborPos]);
                openCosts[neighborPos] = nodes[neighborPos].fCost();
            }
            else if (tentativeG < it->second.gCost)
            {
                it->second.gCost = tentativeG;
                it->second.parentPos = current.pos;
                if (!closedSet.count(neighborPos))
                {
                    openCosts[neighborPos] = it->second.fCost();
                    openHeap.push(it->second);
                }
            }
        }
    }

    PATHFINDER_LOG("No path found. Iterations: " << iterations);
    return {};
}