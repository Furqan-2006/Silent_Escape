#include "mapLoader.hpp"
#include <fstream>
#include <sstream>
#include <SFML/Graphics.hpp>

std::vector<GameObject> loadMap(const std::string &fileName, float tileSize)
{
    std::vector<GameObject> objects;
    std::ifstream file(fileName);
    std::string line;
    int row =0;
    float radius = 25.f;
    int points = 10;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        int col = 0, tile;
        while (ss >> tile)
        {
            sf::Vector2f pos(col * tileSize, row * tileSize);
            switch (tile)
            {
            case 1: // Wall
                objects.emplace_back(radius, points, pos, sf::Color::Blue, ObjectType::Wall);
                break;
            case 2: // Box
                objects.emplace_back(radius, points, pos, sf::Color::Yellow, ObjectType::Box);
                break;
            case 3: // Door
                objects.emplace_back(radius, points, pos, sf::Color::Green, ObjectType::Door);
                break;
            }
            col++;
        }
        row++;
    }
    return objects;
}