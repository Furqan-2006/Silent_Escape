#pragma once
#include <SFML/Graphics.hpp>

inline sf::Vector2f toIsometric(sf::Vector2i gridPos, int tileSize)
{

    if (tileSize == 0)
    {
        std::cerr << "[Error] tileSize is 0 in toIsometric!" << std::endl;
        return {0.f, 0.f};
    }
    int tileWidth = tileSize, tileHeight = tileSize / 2.f;
    sf::Vector2f offSet(1000, 100);

    float x = (gridPos.x - gridPos.y) * (tileWidth / 2.f);
    float y = (gridPos.x + gridPos.y) * (tileHeight / 2.f);

    sf::Vector2f newPosition(x + offSet.x, y + offSet.y);

    return newPosition;
}
inline sf::Vector2f toIsometricDir(sf::Vector2f dir, float tileSize)
{
    return {
        (dir.x - dir.y),
        (dir.x + dir.y)};
}
