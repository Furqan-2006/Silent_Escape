#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

inline sf::Vector2f toIsometric(sf::Vector2i gridPos, int tileSize, sf::Vector2f offSet = {1000.f, 100.f})
{
    if (tileSize == 0)
    {
        std::cerr << "[Error] tileSize is 0 in toIsometric!" << std::endl;
        return {0.f, 0.f};
    }
    float tileWidth = static_cast<float>(tileSize);
    float tileHeight = tileWidth / 2.f;

    float x = (gridPos.x - gridPos.y) * (tileWidth / 2.f);
    float y = (gridPos.x + gridPos.y) * (tileHeight / 2.f);

    return {x + offSet.x, y + offSet.y};
}

inline sf::Vector2f toIsometricDir(sf::Vector2f dir, float tileSize)
{
    float tileWidth = tileSize;
    float tileHeight = tileSize / 2.f;

    return {
        (dir.x - dir.y) * (tileWidth / 2.f),
        (dir.x + dir.y) * (tileHeight / 2.f)};
}
