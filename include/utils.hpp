#pragma once
#include <SFML/Graphics.hpp>

inline sf::Vector2f toIsometric(sf::Vector2i gridPos, int tileWidth, int tileHeight)
{
    float x = (gridPos.x - gridPos.y) * (tileWidth / 2.f);
    float y = (gridPos.x + gridPos.y) * (tileHeight / 2.f);
    return {x, y};
}
