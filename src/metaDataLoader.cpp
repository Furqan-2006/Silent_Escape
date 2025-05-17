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
    if (data.contains("goal"))
    {
        metadata.goalPos.x = data["goal"]["x"].get<int>();
        metadata.goalPos.y = data["goal"]["y"].get<int>();
    }

    if (data.contains("player"))
    {
        metadata.playerPos.x = data["player"]["x"].get<float>() * tileSize;
        metadata.playerPos.y = data["player"]["y"].get<float>() * tileSize;
        metadata.playerGridPos.x = data["player"]["x"].get<int>();
        metadata.playerGridPos.y = data["player"]["y"].get<int>();
    }

    if (data.contains("guards"))
    {
        for (const auto &g : data["guards"])
        {
            GuardMetadata guard;
            guard.position.x = g["x"].get<float>() * tileSize;
            guard.position.y = g["y"].get<float>() * tileSize;
            guard.gridPos.x = g["x"].get<int>();
            guard.gridPos.y = g["y"].get<int>();

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
                    float px = point["x"].get<int>();
                    float py = point["y"].get<int>();
                    guard.addPoint(toIsometric(sf::Vector2i(px, py), tileSize));
                }
            }
            metadata.guards.push_back(guard);
        }
    }
    return metadata;
}
