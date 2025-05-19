#include "PathFinder.hpp"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>

#ifdef DEBUG_PATHFINDER
#define PATHFINDER_LOG(x) std::cout << "[PathFinder] " << x << "\n" // Log pathfinding info if DEBUG_PATHFINDER is defined
#else
#define PATHFINDER_LOG(x)
#endif

// Convert pixel coordinates to grid coordinates
sf::Vector2i pixelToGrid(const sf::Vector2f &pos, float tileSize)
{
    return sf::Vector2i(static_cast<int>(pos.x / tileSize), static_cast<int>(pos.y / tileSize));
}

// Convert grid coordinates to pixel coordinates (center of the tile)
sf::Vector2f gridToPixel(const sf::Vector2i &gridPos, float tileSize)
{
    return sf::Vector2f(gridPos.x * tileSize + tileSize / 2.f, gridPos.y * tileSize + tileSize / 2.f);
}

// Node structure used in A* algorithm to represent positions on the grid
struct PathFinder::Node
{
    sf::Vector2i pos, parentPos; // Position and parent position of the node
    float gCost, hCost;          // gCost (cost from start) and hCost (heuristic to goal)
    Node(sf::Vector2i position = {}, sf::Vector2i parent = {}, float g = 0.f, float h = 0.f)
        : pos(position), parentPos(parent), gCost(g), hCost(h) {}

    // fCost = gCost + hCost (total cost for A* algorithm)
    float fCost() const { return gCost + hCost; }
};

// Comparator for priority queue to order nodes by their fCost (lowest first)
struct PathFinder::NodeComparator
{
    bool operator()(const Node &a, const Node &b) const
    {
        return a.fCost() > b.fCost(); // Compare nodes based on fCost (lower fCost has higher priority)
    }
};

// Hash function for sf::Vector2i (used in unordered_map)
struct PathFinder::Vector2iHash
{
    std::size_t operator()(const sf::Vector2i &v) const noexcept
    {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1); // Combine hashes of x and y
    }
};

// Equality function for sf::Vector2i (used in unordered_set)
struct PathFinder::Vector2iEq
{
    bool operator()(const sf::Vector2i &a, const sf::Vector2i &b) const noexcept
    {
        return a.x == b.x && a.y == b.y; // Check if two vector positions are equal
    }
};

// Constructor for PathFinder, initializing grid, world size, map data, etc.
PathFinder::PathFinder(float stepSize, int worldWidth, int worldHeight, const std::vector<std::vector<int>> &mapData, float tileSize)
    : step(stepSize), width(worldWidth), height(worldHeight), tileSize(tileSize), mapData(mapData)
{
    // Initialize the grid with walkable/blocked values based on map data
    grid.resize(width, std::vector<bool>(height, true)); // Default to all walkable
    for (int y = 0; y < height && y < mapData.size(); ++y)
        for (int x = 0; x < width && x < mapData[y].size(); ++x)
            grid[x][y] = mapData[y][x] == 0; // 0 = walkable, 1 = blocked

    // Log pathfinding initialization details
    PATHFINDER_LOG("Initialized with stepSize=" << stepSize << ", worldSize=(" << worldWidth << "x" << worldHeight << "), tileSize=" << tileSize);
}

// Update the grid to account for obstacles (walls, boxes, doors, etc.)
void PathFinder::updateObstacles(const std::vector<GameObject> &obstacles)
{
    // Reset the grid to default walkable values based on map data
    for (int y = 0; y < height && y < mapData.size(); ++y)
        for (int x = 0; x < width && x < mapData[y].size(); ++x)
            grid[x][y] = mapData[y][x] == 0;

    // Update grid with obstacles' positions
    for (const auto &obj : obstacles)
    {
        if (obj.getType() == ObjectType::Wall || obj.getType() == ObjectType::Box || obj.getType() == ObjectType::Door)
        {
            sf::FloatRect bounds = obj.getBounds(); // Get the bounds of the object
            if (bounds.size.x <= 0 || bounds.size.y <= 0)
                continue; // Skip if object has invalid size

            // Adjust bounds for circular shapes (shrink the bounds for better collision detection)
            float radius = bounds.size.x / 2.f;
            float shrinkFactor = 0.7f; // Shrink factor to reduce overestimation
            bounds.position.x += radius * (1 - shrinkFactor);
            bounds.position.y += radius * (1 - shrinkFactor);
            bounds.size.x *= shrinkFactor;
            bounds.size.y *= shrinkFactor;

            // Calculate the grid coordinates covered by the obstacle and mark them as non-walkable
            int minX = std::max(0, static_cast<int>(bounds.position.x / tileSize));
            int maxX = std::min(width - 1, static_cast<int>((bounds.position.x + bounds.size.x) / tileSize));
            int minY = std::max(0, static_cast<int>(bounds.position.y / tileSize));
            int maxY = std::min(height - 1, static_cast<int>((bounds.position.y + bounds.size.y) / tileSize));

            // Mark the grid cells covered by the obstacle as non-walkable
            for (int x = minX; x <= maxX; ++x)
                for (int y = minY; y <= maxY; ++y)
                    grid[x][y] = false;
        }
    }
}

// Round the pixel position to the center of the corresponding grid tile
sf::Vector2f PathFinder::roundToTileCenter(const sf::Vector2f &pixelPos) const
{
    int tileX = static_cast<int>(pixelPos.x / tileSize);
    int tileY = static_cast<int>(pixelPos.y / tileSize);
    sf::Vector2f rounded(tileX * tileSize + tileSize / 2.f, tileY * tileSize + tileSize / 2.f); // Round to the center of the tile
    PATHFINDER_LOG("roundToTileCenter: (" << pixelPos.x << "," << pixelPos.y << ") -> (" << rounded.x << "," << rounded.y << ")");
    return rounded;
}

// Check if a point is walkable (i.e., not blocked by obstacles)
bool PathFinder::isWalkable(const sf::Vector2f &point) const
{
    int tileX = static_cast<int>(point.x / tileSize);
    int tileY = static_cast<int>(point.y / tileSize);
    bool walkable = tileX >= 0 && tileX < width && tileY >= 0 && tileY < height && grid[tileX][tileY]; // Check if the tile is within bounds and walkable
    PATHFINDER_LOG("isWalkable: (" << point.x << "," << point.y << ") -> tile (" << tileX << "," << tileY << ") -> " << (walkable ? "WALKABLE" : "BLOCKED"));
    return walkable;
}

// Check if a straight line between two points is walkable (used for line-of-sight checks or direct pathfinding)
bool PathFinder::isLineWalkable(const sf::Vector2f &start, const sf::Vector2f &end) const
{
    sf::Vector2f dir = end - start;
    float distance = hypot(dir.x, dir.y); // Calculate the Euclidean distance between the points
    if (distance == 0)
        return true; // If the distance is 0, the path is trivially walkable
    dir /= distance;

    // Check each point along the line to see if it's walkable
    for (float t = 0; t <= distance; t += tileSize / 4.f) // Check at intervals of tileSize/4 (fine-grained check)
    {
        sf::Vector2f point = start + dir * t;
        if (!isWalkable(point))
            return false; // Return false if any point along the line is not walkable
    }
    return true;
}

// Smooth the path by removing unnecessary points if the path is straight
std::vector<sf::Vector2f> PathFinder::smoothPath(const std::vector<sf::Vector2f> &path) const
{
    if (path.size() < 2)
        return path; // No need to smooth if the path has less than 2 points

    std::vector<sf::Vector2f> smoothed; // Vector to store the smoothed path
    smoothed.push_back(path[0]);        // Add the first point to the smoothed path

    for (size_t i = 1; i < path.size() - 1; ++i)
    {
        sf::Vector2f start = smoothed.back();
        sf::Vector2f end = path[i + 1];
        if (isLineWalkable(start, end)) // If the line between the current and next point is walkable, skip this point
            continue;
        smoothed.push_back(path[i]); // Otherwise, add the current point to the smoothed path
    }
    smoothed.push_back(path.back()); // Add the last point to the smoothed path

    PATHFINDER_LOG("Smoothed path from " << path.size() << " to " << smoothed.size() << " points");
    return smoothed; // Return the smoothed path
}

// A* pathfinding algorithm to find the shortest path from start to goal
std::vector<sf::Vector2f> PathFinder::findPath(sf::Vector2f start, sf::Vector2f goal)
{
    // Convert start and goal from pixel to grid coordinates
    sf::Vector2i startGrid = pixelToGrid(start, tileSize);
    sf::Vector2i goalGrid = pixelToGrid(goal, tileSize);

    PATHFINDER_LOG("START Pathfinding from (" << start.x << ", " << start.y << ") to (" << goal.x << ", " << goal.y << ")");

    // Round start and goal to tile centers in pixels (for walkability checks)
    sf::Vector2f roundedStart = roundToTileCenter(start);
    sf::Vector2f roundedGoal = roundToTileCenter(goal);

    if (!isWalkable(roundedStart))
    {
        PATHFINDER_LOG("ERROR: Start position is not walkable!");
        return {}; // Return an empty path if the start position is not walkable
    }

    if (!isWalkable(roundedGoal))
    {
        // Try nearby tiles to adjust goal to walkable tile
        const std::array<sf::Vector2f, 4> offsets = {
            sf::Vector2f(tileSize, 0),
            sf::Vector2f(-tileSize, 0),
            sf::Vector2f(0, tileSize),
            sf::Vector2f(0, -tileSize)};
        bool foundWalkable = false;
        for (const auto &offset : offsets)
        {
            sf::Vector2f newGoal = roundedGoal + offset;
            if (isWalkable(newGoal))
            {
                roundedGoal = newGoal;
                goalGrid = pixelToGrid(newGoal, tileSize);
                PATHFINDER_LOG("Adjusted goal to walkable point: (" << roundedGoal.x << ", " << roundedGoal.y << ")");
                foundWalkable = true;
                break;
            }
        }
        if (!foundWalkable)
        {
            PATHFINDER_LOG("ERROR: Goal position is not walkable!");
            return {}; // Return an empty path if no walkable goal is found
        }
    }

    // If start and goal are close and direct path is walkable, shortcut and return directly
    if (hypot(roundedGoal.x - roundedStart.x, roundedGoal.y - roundedStart.y) < tileSize * 2 && isLineWalkable(roundedStart, roundedGoal))
    {
        PATHFINDER_LOG("Direct path used for nearby target");
        return {roundedStart, roundedGoal}; // Return a direct path if the goal is nearby and walkable
    }

    // Directions to check (grid offsets)
    std::vector<sf::Vector2i> directions = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    // Helper lambda: heuristic using Euclidean distance in pixel coords
    auto heuristic = [&](const sf::Vector2i &a, const sf::Vector2i &b) -> float
    {
        sf::Vector2f pa = gridToPixel(a, tileSize);
        sf::Vector2f pb = gridToPixel(b, tileSize);
        return hypot(pb.x - pa.x, pb.y - pa.y); // Heuristic based on Euclidean distance between two grid positions
    };

    // Priority queue to store open nodes (nodes to be evaluated)
    std::unordered_map<sf::Vector2i, Node, Vector2iHash, Vector2iEq> nodes;
    std::unordered_map<sf::Vector2i, float, Vector2iHash, Vector2iEq> openCosts;
    std::unordered_set<sf::Vector2i, Vector2iHash, Vector2iEq> closedSet;
    std::priority_queue<Node, std::vector<Node>, NodeComparator> openHeap;

    nodes[startGrid] = Node(startGrid, startGrid, 0.f, heuristic(startGrid, goalGrid));
    openHeap.push(nodes[startGrid]);
    openCosts[startGrid] = nodes[startGrid].fCost();

    int iterations = 0;
    const int maxIterations = 10000;

    // A* algorithm main loop
    while (!openHeap.empty() && iterations++ < maxIterations)
    {
        Node current = openHeap.top();
        openHeap.pop();
        openCosts.erase(current.pos);

        if (closedSet.count(current.pos)) // Skip if the node has already been processed
            continue;

        if (current.pos == goalGrid)
        {
            // Reconstruct path (in grid coords)
            std::vector<sf::Vector2f> path;
            for (sf::Vector2i pos = goalGrid; pos != startGrid; pos = nodes[pos].parentPos)
                path.push_back(gridToPixel(pos, tileSize));
            path.push_back(gridToPixel(startGrid, tileSize));
            std::reverse(path.begin(), path.end());

            // Smooth path in pixel space
            path = smoothPath(path);

            PATHFINDER_LOG("COMPLETE. Path length: " << path.size() << ", iterations: " << iterations);
            return path;
        }

        closedSet.insert(current.pos); // Mark the current node as processed

        // Evaluate neighbors of the current node
        for (const auto &dir : directions)
        {
            sf::Vector2i neighborPos = current.pos + dir;
            sf::Vector2f neighborPixelPos = gridToPixel(neighborPos, tileSize);

            if (!isWalkable(neighborPixelPos) || closedSet.count(neighborPos))
                continue;

            float moveCost = hypot(dir.x, dir.y);
            float tentativeG = current.gCost + moveCost;

            auto it = nodes.find(neighborPos);
            if (it == nodes.end())
            {
                nodes[neighborPos] = Node(neighborPos, current.pos, tentativeG, heuristic(neighborPos, goalGrid));
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
    return {}; // Return an empty path if no path is found within the max iterations
}
