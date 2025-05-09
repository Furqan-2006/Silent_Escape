#pragma once
#include <SFML/Graphics.hpp>

inline sf::Vector2f toIsometric(sf::Vector2i gridPos, int tileSize)
{
    if (tileSize == 0)
    {
        std::cerr << "[Error] tileSize is 0 in toIsometric!" << std::endl;
        return {0.f, 0.f};
    }
    int tileWidth = tileSize, tileHeight = tileSize;

    // float x = (gridPos.x - gridPos.y) * (tileWidth / 2.f) ;
    // float y = (gridPos.x + gridPos.y) * (tileHeight /2.f);

    float x = (gridPos.x * tileWidth * 0.5) + (gridPos.y * tileWidth * (0.5));
    float y = (gridPos.x * tileWidth * 0.25) + (gridPos.y * tileWidth * 0.25);
    // float x = (gridPos.x * tileWidth ) + (gridPos.y * tileWidth  );
    // float y = (gridPos.x * tileWidth ) + (gridPos.y * tileWidth );

    return {x, y};
}
