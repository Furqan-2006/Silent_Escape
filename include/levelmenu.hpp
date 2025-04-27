#pragma once

#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>

#define NO_OF_LEVELS 3

class LevelTile
{
    sf::RectangleShape tile;

    bool isSelected = false;
    bool isLocked = true;

public:
    LevelTile(float x, float y, float size);

    void draw(sf::RenderWindow &window);

    bool contains(sf::Vector2f &point);
    void select();
    void deSelect();

    void unlock();
    bool getIsLocked() const;
};

class LevelMenu
{
    int selectedLevelIndex ;
    std::vector<LevelTile> tiles;
    std::vector<sf::Text> tileLabels;
    sf::Font font;

public:
    LevelMenu(float width, float height);

    void draw(sf::RenderWindow &window);

    // void moveUp();
    // void moveDown();
    void moveRight();
    void moveLeft();
    void onClick(sf::Vector2f mousePos);

    int getSelectedLevel() const;
};
