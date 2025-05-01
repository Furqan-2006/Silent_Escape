#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>

class IsometricConverter
{
private:
    float tileWidth_;
    float tileHeight_;

public:
    IsometricConverter(float tileWidth = 40.0f, float tileHeight = 20.0f)
        : tileWidth_(tileWidth), tileHeight_(tileHeight) {}

    // Convert Cartesian (top-down) to isometric coordinates
    sf::Vector2f cartesianToIsometric(const sf::Vector2f &cartesian) const
    {
        return sf::Vector2f(
            (cartesian.x - cartesian.y) * tileWidth_ / 2,
            (cartesian.x + cartesian.y) * tileHeight_ / 4);
    }

    // Convert isometric to Cartesian coordinates
    sf::Vector2f isometricToCartesian(const sf::Vector2f &iso) const
    {
        float x = (iso.x / tileWidth_ + iso.y / tileHeight_) * 2;
        float y = (iso.y / tileHeight_ - iso.x / tileWidth_) * 2;
        return sf::Vector2f(x, y);
    }

    sf::Vector2f getIsometricScale() const
    {
        return sf::Vector2f(1.0f, 0.5f);
    }

    float getDepth(const sf::Vector2f &isoPos) const
    {
        return isoPos.y;
    }
};