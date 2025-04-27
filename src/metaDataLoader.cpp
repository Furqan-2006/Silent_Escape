#include "metaDataLoader.hpp"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

LevelMetadata loadMetadata(const std::string &jsonPath, float tileSize)
{
    LevelMetadata metadata;
    std::ifstream file(jsonPath);
    if (!file.is_open())
    {
        std::cerr << "Could'nt open metadata file: " << jsonPath << std::endl;
        return metadata;
    }
    json data;
    file >> data;
    if (data.contains("player"))
    {
        metadata.playerPos.x = data["player"]["x"].get<float>() * tileSize;
        metadata.playerPos.y = data["player"]["y"].get<float>() * tileSize;
    }

    if (data.contains("guards"))
    {
        for (const auto &g : data["guards"])
        {
            GuardMetadata guard;
            guard.position.x = g["x"].get<float>() * tileSize;
            guard.position.y = g["y"].get<float>() * tileSize;

            if (g.contains("dx") && g.contains("dy"))
            {
                guard.direction = {g["dx"].get<float>(), g["dy"].get<float>()};
            }
            else
            {
                guard.direction = {0.f, 0.f};
            }
            if (g.contains("patrolPath"))
            {
                for (const auto &point : g["patrolPath"])
                {
                    float px = point["x"].get<float>() * tileSize / 2.f;
                    float py = point["y"].get<float>() * tileSize / 2.f;
                    guard.addPoint(sf::Vector2f(px, py));
                }
            }
            metadata.guards.push_back(guard);
        }
    }
    return metadata;
}
