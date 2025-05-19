#include "mapLoader.hpp"
#include <fstream>
#include <sstream>
#include <SFML/Graphics.hpp>

// Function to load the map from a file and create game objects based on the map data
std::vector<GameObject> loadMap(const std::string &fileName, TextureManager &textureManager, float tileSize)
{
    std::vector<GameObject> objects; // Vector to store the created game objects
    std::ifstream file(fileName);    // Open the file containing map data
    std::string line;                // To store each line of the map file
    int row = 0;                     // Row counter for the map
    float radius = 20.f;             // Radius for certain objects (unused in the current code)
    int points = 10;                 // Points for certain objects (unused in the current code)

    // Loop through each line in the file
    while (std::getline(file, line))
    {
        std::stringstream ss(line); // Create a stringstream from the line to read individual tiles
        int col = 0, tile;          // Column counter and tile type
        // Loop through each tile in the line (separated by spaces)
        while (ss >> tile)
        {
            sf::Vector2f pos(col * tileSize, row * tileSize); // Calculate the position of the tile based on row and column
            // Check for invalid (negative) positions
            if (pos.x < 0 || pos.y < 0)
            {
                std::cout << "[Warning] Negative position: (" << pos.x << ", " << pos.y << ") at row " << row << ", col " << col << std::endl;
            }

            // Based on the tile value, create the corresponding game object
            switch (tile)
            {
            case 9: // Wall (horizontal)
                objects.emplace_back(textureManager.get("wall-LR"), pos, ObjectType::Wall, sf::Vector2i({row, col}), tileSize);
                break;
            case 1: // Wall (vertical)
                objects.emplace_back(textureManager.get("wall-TB"), pos, ObjectType::Wall, sf::Vector2i({row, col}), tileSize);
                break;
                // Case for other objects (currently commented out, but can be added)
                // case 2: // Box
                //     objects.emplace_back(textureManager.get("box"), pos, ObjectType::Box, sf::Vector2i({row, col}), tileSize);
                //     break;
                // case 3: // Door
                //     objects.emplace_back(textureManager.get("door"), pos, ObjectType::Door, sf::Vector2i({row, col}), tileSize);
                //     break;
                // case 4: // Disguise
                //     objects.emplace_back(textureManager.get("disguise"), pos, ObjectType::Disguise, sf::Vector2i({row, col}), tileSize);
                //     break;
                // case 5: // Terminal
                //     objects.emplace_back(textureManager.get("terminal"), pos, ObjectType::Terminal, sf::Vector2i({row, col}), tileSize);
                // break;
            }
            col++; // Increment column counter
        }
        row++; // Increment row counter
    }
    return objects; // Return the vector of created game objects
}

// Function to load a grid map that indicates walkability of each tile (1 for non-walkable, 0 for walkable)
std::vector<std::vector<int>> loadGridMap(const std::string &fileName)
{
    std::vector<std::vector<int>> grid; // 2D vector to store the grid map
    std::ifstream file(fileName);       // Open the file containing map data
    std::string line;                   // To store each line of the map file

    // Loop through each line in the file
    while (std::getline(file, line))
    {
        std::stringstream ss(line); // Create a stringstream from the line to read individual tiles
        std::vector<int> row;       // A row to store the tile data for each line
        int tile;                   // Tile type (either 0 or 1)
        // Loop through each tile in the line (separated by spaces)
        while (ss >> tile)
        {
            // Mark as 1 (non-walkable) for walls, boxes, doors, etc.
            // Mark as 0 (walkable) for floors, disguises, terminals, etc.
            if (tile == 1 || tile == 9) // 1 and 9 represent non-walkable tiles (walls)
                row.push_back(1);       // Mark as non-walkable
            else
                row.push_back(0); // Mark as walkable
        }
        grid.push_back(row); // Add the row to the grid
    }

    return grid; // Return the grid map
}
