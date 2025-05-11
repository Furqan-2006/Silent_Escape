#include "mapLoader.hpp"
#include <fstream>
#include <sstream>
#include <SFML/Graphics.hpp>

std::vector<GameObject> loadMap(const std::string &fileName, TextureManager &textureManager, float tileSize)
{
    std::vector<GameObject> objects;
    std::ifstream file(fileName);
    std::string line;
    int row = 0;
    float radius = 20.f;
    int points = 10;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        int col = 0, tile;
        while (ss >> tile)
        {
            sf::Vector2f pos(col * tileSize, row * tileSize);
            if (pos.x < 0 || pos.y < 0)
            {
                std::cout << "[Warning] Negative position: (" << pos.x << ", " << pos.y << ") at row " << row << ", col " << col << std::endl;
            }

            switch (tile)
            {
            case 9: // Wall
                objects.emplace_back(textureManager.get("wall-LR"), pos, ObjectType::Wall, sf::Vector2i({row, col}), tileSize);
                break;
            case 1: // Wall
                objects.emplace_back(textureManager.get("wall-TB"), pos, ObjectType::Wall, sf::Vector2i({row, col}), tileSize);
                break;
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
            col++;
        }
        row++;
    }
    return objects;
}

std::vector<std::vector<int>> loadGridMap(const std::string &fileName)
{
    std::vector<std::vector<int>> grid;
    std::ifstream file(fileName);
    std::string line;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::vector<int> row;
        int tile;
        while (ss >> tile)
        {
            // Mark as 1 (non-walkable) for walls, box, door
            // Mark as 0 (walkable) for floor, disguise, terminal, etc.
            if (tile == 1 || tile == 2 || tile == 3)
                row.push_back(1);
            else
                row.push_back(0);
        }
        grid.push_back(row);
    }

    return grid;
}
