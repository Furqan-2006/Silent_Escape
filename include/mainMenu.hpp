#pragma once

#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>

#define MAX_ITEMS 3

class Menu
{
private:
    int selectedItemIndex;
    sf::Font font;
    std::vector<sf::Text> menu;

public:
    Menu(float width, float height);
    void draw(sf::RenderWindow &window);
    void moveUp();
    void moveDown();
    int getSelectedIndex() const;
};
