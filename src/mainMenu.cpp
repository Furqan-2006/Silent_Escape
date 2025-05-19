#include "mainMenu.hpp"

// Constructor to initialize the main menu
Menu::Menu(float width, float height)
{
    // Try to load the font from the specified path
    if (!font.openFromFile("../assets/Fonts/CURLZ___.TTF"))
    {
        std::cerr << "Error: Failed to load font from assets/Fonts/CURLZ___.TTF\n"; // Error handling if the font fails to load
    }

    // Define the labels for the menu items
    std::string labels[MAX_ITEMS] = {"Start Game", "Options", "Exit"};

    // Reserve space for menu items
    menu.reserve(MAX_ITEMS);

    // Create and initialize the menu items (text)
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        sf::Text text(font, labels[i], 28);                                      // Create text with the font, label, and size
        text.setFillColor(i == 0 ? sf::Color::Green : sf::Color::White);         // Set the color of the selected item to Green, others to White
        text.setPosition({width / 2 - 100, height / (MAX_ITEMS + 1) * (i + 1)}); // Position the text in the center of the window

        menu.push_back(text); // Add the created text to the menu
    }

    selectedItemIndex = 0; // Set the initial selected item to the first menu item
}

// Draw the menu items to the window
void Menu::draw(sf::RenderWindow &window)
{
    for (const auto &text : menu) // Loop through all menu items
    {
        window.draw(text); // Draw each item to the window
    }
}

// Move the selection up in the menu
void Menu::moveUp()
{
    if (selectedItemIndex > 0) // Ensure the selection index is not out of bounds
    {
        menu[selectedItemIndex].setFillColor(sf::Color::White); // Deselect the current item (set its color to white)
        selectedItemIndex--;                                    // Move the selection up (decrease the index)
        menu[selectedItemIndex].setFillColor(sf::Color::Green); // Select the new item (set its color to green)
    }
}

// Move the selection down in the menu
void Menu::moveDown()
{
    if (selectedItemIndex < static_cast<int>(menu.size()) - 1) // Ensure the selection index is within bounds
    {
        menu[selectedItemIndex].setFillColor(sf::Color::White); // Deselect the current item (set its color to white)
        selectedItemIndex++;                                    // Move the selection down (increase the index)
        menu[selectedItemIndex].setFillColor(sf::Color::Green); // Select the new item (set its color to green)
    }
}

// Get the index of the currently selected menu item
int Menu::getSelectedIndex() const
{
    return selectedItemIndex; // Return the index of the selected menu item
}
