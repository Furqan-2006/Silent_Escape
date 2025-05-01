#include "PathFinder.hpp"
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <limits>
#include <iostream> // For debug logging

// Node struct represents a point in the search.
struct PathFinder::Node
{
    sf::Vector2f pos;
    Node *parent;
    float gCost, hCost;

    Node(sf::Vector2f position = {}, Node *parentNode = nullptr)
        : pos(position), parent(parentNode), gCost(0), hCost(0) {}

    float fCost() const { return gCost + hCost; }
};

// Comparator for priority queue (min-heap) based on fCost.
struct PathFinder::NodeComparator
{
    bool operator()(const Node *a, const Node *b) const
    {
        return a->fCost() > b->fCost();
    }
};

// Hash and equality for sf::Vector2f.
struct PathFinder::Vector2fHash
{
    std::size_t operator()(const sf::Vector2f &v) const noexcept
    {
        auto h1 = std::hash<int>()(static_cast<int>(v.x));
        auto h2 = std::hash<int>()(static_cast<int>(v.y));
        return h1 ^ (h2 << 1);
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
    : step(stepSize), width(worldWidth), height(worldHeight), tileSize(tileSize)
{
    std::cout << "[PathFinder] Initialized with stepSize=" << stepSize
              << ", worldSize=(" << worldWidth << "x" << worldHeight
              << "), tileSize=" << tileSize << "\n";
}

// Round any pixel position to the center of its tile
sf::Vector2f PathFinder::roundToTileCenter(const sf::Vector2f &pixelPos) const
{
    int tileX = static_cast<int>(pixelPos.x) / tileSize;
    int tileY = static_cast<int>(pixelPos.y) / tileSize;
    sf::Vector2f rounded(tileX * tileSize + tileSize / 2.f, tileY * tileSize + tileSize / 2.f);

    std::cout << "[roundToTileCenter] (" << pixelPos.x << "," << pixelPos.y
              << ") -> (" << rounded.x << "," << rounded.y << ")\n";
    return rounded;
}

// Check if a pixel position is walkable by checking its tile
bool PathFinder::isWalkable(const sf::Vector2f &point) const
{
    int tileX = static_cast<int>(point.x) / tileSize;
    int tileY = static_cast<int>(point.y) / tileSize;

    if (tileX < 0 || tileX >= width || tileY < 0 || tileY >= height)
    {
        std::cout << "[isWalkable] (" << point.x << "," << point.y << ") -> OUT OF BOUNDS\n";
        return false;
    }

    // Replace this with your real collision logic (example: tileMap[tileY][tileX] != WALL)
    bool walkable = true; // Placeholder: every tile is walkable
    std::cout << "[isWalkable] (" << point.x << "," << point.y << ") -> tile ("
              << tileX << "," << tileY << ") -> " << (walkable ? "WALKABLE" : "BLOCKED") << "\n";
    return walkable;
}

std::vector<sf::Vector2f> PathFinder::findPath(sf::Vector2f start, sf::Vector2f goal)
{
    std::cout << "\n[findPath] START Pathfinding from (" << start.x << ", " << start.y
              << ") to (" << goal.x << ", " << goal.y << ")\n";

    // Round start and goal to tile centers
    start = roundToTileCenter(start);
    goal = roundToTileCenter(goal);

    std::cout << "[findPath] Rounded positions - start: (" << start.x << "," << start.y
              << "), goal: (" << goal.x << "," << goal.y << ")\n";

    if (!isWalkable(start))
    {
        std::cerr << "[findPath] ERROR: Start position is not walkable!\n";
        return {};
    }

    if (!isWalkable(goal))
    {
        std::cerr << "[findPath] ERROR: Goal position is not walkable!\n";
        return {};
    }

    const std::vector<sf::Vector2f> directions = {
        {step, 0}, {-step, 0}, {0, step}, {0, -step}, {step, step}, {step, -step}, {-step, step}, {-step, -step}};

    std::unordered_map<sf::Vector2f, Node *, Vector2fHash, Vector2fEq> allNodes;
    std::priority_queue<Node *, std::vector<Node *>, NodeComparator> openHeap;
    std::unordered_map<sf::Vector2f, bool, Vector2fHash, Vector2fEq> closedSet;

    Node *startNode = new Node(start, nullptr);
    startNode->gCost = 0;
    startNode->hCost = hypot(goal.x - start.x, goal.y - start.y);
    allNodes[start] = startNode;
    openHeap.push(startNode);

    std::cout << "[findPath] Initial node - pos: (" << startNode->pos.x << "," << startNode->pos.y
              << "), gCost: " << startNode->gCost << ", hCost: " << startNode->hCost
              << ", fCost: " << startNode->fCost() << "\n";

    Node *goalNode = nullptr;
    int iterations = 0;
    int maxIterations = 10000; // Safety limit

    while (!openHeap.empty() && iterations++ < maxIterations)
    {
        Node *current = openHeap.top();
        openHeap.pop();

        std::cout << "\n[findPath] Processing node #" << iterations
                  << " - pos: (" << current->pos.x << "," << current->pos.y
                  << "), fCost: " << current->fCost() << " (g=" << current->gCost
                  << ", h=" << current->hCost << ")\n";

        if (closedSet[current->pos])
        {
            std::cout << "[findPath] Node already in closed set, skipping\n";
            continue;
        }

        if (current->pos == goal)
        {
            std::cout << "[findPath] GOAL REACHED!\n";
            goalNode = current;
            break;
        }

        closedSet[current->pos] = true;
        std::cout << "[findPath] Added to closed set. Closed set size: " << closedSet.size() << "\n";

        for (const sf::Vector2f &dir : directions)
        {
            sf::Vector2f neighborPos = current->pos + dir;
            std::cout << "[findPath] Checking neighbor at (" << neighborPos.x << "," << neighborPos.y << ")\n";

            if (!isWalkable(neighborPos))
            {
                std::cout << "[findPath] Neighbor not walkable, skipping\n";
                continue;
            }

            float moveCost = hypot(dir.x, dir.y);
            float tentativeG = current->gCost + moveCost;

            Node *neighborNode;
            auto it = allNodes.find(neighborPos);
            if (it == allNodes.end())
            {
                neighborNode = new Node(neighborPos, current);
                neighborNode->gCost = tentativeG;
                neighborNode->hCost = hypot(goal.x - neighborPos.x, goal.y - neighborPos.y);
                allNodes[neighborPos] = neighborNode;
                openHeap.push(neighborNode);

                std::cout << "[findPath] New neighbor node - pos: (" << neighborPos.x << "," << neighborPos.y
                          << "), gCost: " << tentativeG << ", hCost: " << neighborNode->hCost
                          << ", fCost: " << neighborNode->fCost() << "\n";
            }
            else
            {
                neighborNode = it->second;
                if (tentativeG >= neighborNode->gCost)
                {
                    std::cout << "[findPath] Existing neighbor has better path (current gCost: "
                              << neighborNode->gCost << " <= tentative: " << tentativeG << "), skipping\n";
                    continue;
                }

                neighborNode->gCost = tentativeG;
                neighborNode->parent = current;
                openHeap.push(neighborNode);

                std::cout << "[findPath] Updated neighbor node - pos: (" << neighborPos.x << "," << neighborPos.y
                          << "), new gCost: " << tentativeG << ", hCost: " << neighborNode->hCost
                          << ", fCost: " << neighborNode->fCost() << "\n";
            }
        }
    }

    if (iterations >= maxIterations)
    {
        std::cerr << "[findPath] WARNING: Exceeded maximum iterations (" << maxIterations << ")\n";
    }

    std::vector<sf::Vector2f> path;
    if (goalNode)
    {
        std::cout << "[findPath] Reconstructing path...\n";
        for (Node *node = goalNode; node != nullptr; node = node->parent)
        {
            path.push_back(node->pos);
            std::cout << "  - Path point: (" << node->pos.x << "," << node->pos.y << ")\n";
        }
        std::reverse(path.begin(), path.end());
    }
    else
    {
        std::cout << "[findPath] No path found to goal\n";
    }

    // Cleanup
    for (auto &pair : allNodes)
    {
        delete pair.second;
    }

    std::cout << "[findPath] COMPLETE. Path length: " << path.size()
              << ", Total nodes processed: " << iterations
              << ", Total nodes created: " << allNodes.size() << "\n\n";

    return path;
}