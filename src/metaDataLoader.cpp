#include "metaDataLoader.hpp"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json; // Alias for nlohmann::json for convenience

// Function to load level metadata from a JSON file
LevelMetadata loadMetadata(const std::string &jsonPath, float tileSize)
{
    LevelMetadata metadata;       // Create a LevelMetadata object to store the parsed metadata
    std::ifstream file(jsonPath); // Open the JSON file containing level metadata

    // Check if the file could be opened successfully
    if (!file.is_open())
    {
        std::cerr << "Could'nt open metadata file: " << jsonPath << std::endl; // Print error if file can't be opened
        return metadata;                                                       // Return empty metadata if file couldn't be opened
    }

    json data;    // Create a JSON object to store the contents of the file
    file >> data; // Parse the file contents into the JSON object

    // If the JSON data contains a "goal" entry, extract the goal position
    if (data.contains("goal"))
    {
        metadata.goalPos.x = data["goal"]["x"].get<int>(); // Get the goal's X position
        metadata.goalPos.y = data["goal"]["y"].get<int>(); // Get the goal's Y position
    }

    // If the JSON data contains a "player" entry, extract the player's position and grid position
    if (data.contains("player"))
    {
        metadata.playerPos.x = data["player"]["x"].get<float>() * tileSize; // Convert player's X position to world coordinates using tileSize
        metadata.playerPos.y = data["player"]["y"].get<float>() * tileSize; // Convert player's Y position to world coordinates using tileSize
        metadata.playerGridPos.x = data["player"]["x"].get<int>();          // Store the player's X grid position
        metadata.playerGridPos.y = data["player"]["y"].get<int>();          // Store the player's Y grid position
    }

    // If the JSON data contains a "guards" entry, extract the guards' information
    if (data.contains("guards"))
    {
        // Loop through each guard in the guards array
        for (const auto &g : data["guards"])
        {
            GuardMetadata guard;                               // Create a GuardMetadata object to store the guard's data
            guard.position.x = g["x"].get<float>() * tileSize; // Convert guard's X position to world coordinates using tileSize
            guard.position.y = g["y"].get<float>() * tileSize; // Convert guard's Y position to world coordinates using tileSize
            guard.gridPos.x = g["x"].get<int>();               // Store the guard's X grid position
            guard.gridPos.y = g["y"].get<int>();               // Store the guard's Y grid position

            // If the guard has direction data (dx, dy), extract and store it
            if (g.contains("dx") && g.contains("dy"))
            {
                guard.direction = {g["dx"].get<float>(), g["dy"].get<float>()}; // Set guard's direction (X, Y)
            }
            else
            {
                guard.direction = {0.f, 0.f}; // Default direction (0, 0) if no direction is provided
            }

            // If the guard has a patrol path, extract and store the patrol points
            if (g.contains("patrolPath"))
            {
                for (const auto &point : g["patrolPath"])
                {
                    float px = point["x"].get<int>(); // Get the patrol point's X grid position
                    float py = point["y"].get<int>(); // Get the patrol point's Y grid position
                    // Convert the patrol point to world coordinates using the tile size and add it to the guard's patrol path
                    guard.addPoint(toIsometric(sf::Vector2i(px, py), tileSize));
                }
            }

            metadata.guards.push_back(guard); // Add the guard to the metadata's guards list
        }
    }

    return metadata; // Return the populated metadata
}
