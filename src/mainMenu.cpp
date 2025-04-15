#include "mainMenu.hpp"

Menu::Menu(float width, float height)
{
    if (!font.openFromFile("../assets/Fonts/CURLZ___.TTF"))
    {
        std::cerr << "Error: Failed to load font from assets/Fonts/CURLZ___.TTF\n";
    }
    std::string labels[MAX_ITEMS] = {"Start Game", "Options", "Exit"};

    menu.reserve(MAX_ITEMS);
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        sf::Text text(font, labels[i], 34);
        text.setFillColor(i == 0 ? sf::Color::Green : sf::Color::White);
        text.setPosition({width / 2 - 100, height / (MAX_ITEMS + 1) * (i + 1)});

        menu.push_back(text);
    }
    selectedItemIndex = 0;
}

void Menu::draw(sf::RenderWindow &window)
{
    for (const auto &text : menu)
    {
        window.draw(text);
    }
}

void Menu::moveUp()
{
    if (selectedItemIndex > 0)
    {
        menu[selectedItemIndex].setFillColor(sf::Color::White);
        selectedItemIndex--;
        menu[selectedItemIndex].setFillColor(sf::Color::Green);
    }
}

void Menu::moveDown()
{
    if (selectedItemIndex < static_cast<int>(menu.size()) - 1)
    {
        menu[selectedItemIndex].setFillColor(sf::Color::White);
        selectedItemIndex++;
        menu[selectedItemIndex].setFillColor(sf::Color::Green);
    }
}

int Menu::getSelectedIndex() const
{
    return selectedItemIndex;
}