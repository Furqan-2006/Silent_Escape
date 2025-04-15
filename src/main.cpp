#include <iostream>
#include <SFML/Graphics.hpp>
#include "mainMenu.hpp"

enum class GameState
{
    MENU,
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    EXIT
};

int main()
{
    sf::RenderWindow window(sf::VideoMode({900, 640}), "Game Menu");
    GameState gamestate = GameState::MENU;

    Menu menu(window.getSize().x, window.getSize().y);

    sf::Font font;
    if (!font.openFromFile("../assets/Fonts/CURLZ___.TTF"))
    {
        std::cerr << "Error: Failed to load font from assets/Fonts/CURLZ___.TTF\n";
    }

    sf::Text levelText(font);
    levelText.setCharacterSize(32);
    levelText.setFillColor(sf::Color::White);
    levelText.setString("Silent Agent - Gameplay Placeholder");
    levelText.setPosition({100, 250});

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (gamestate == GameState::MENU)
                {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Up)
                    {
                        menu.moveUp();
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::Down)
                    {
                        menu.moveDown();
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::Enter)
                    {
                        switch (menu.getSelectedIndex())
                        {
                        case 0:
                            gamestate = GameState::LEVEL_1;

                            break;
                        case 1:
                            std::cout << "[LOG] Options Menu Selected (coming soon)" << std::endl;
                            break;
                        case 2:
                            gamestate = GameState::EXIT;
                            window.close();
                            break;

                        default:
                            break;
                        }
                    }
                }
                if (gamestate == GameState::LEVEL_1)
                {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                    {
                        std::cout << "[LOG] Returning to menu form gameplay" << std::endl;
                        gamestate = GameState::MENU;
                    }
                }
            }
        }
        window.clear();

        switch (gamestate)
        {
        case GameState::MENU:
            menu.draw(window);
            break;
        case GameState::LEVEL_1:
            window.draw(levelText);
            break;
        case GameState::EXIT:
            window.close();
            break;
        }

        window.display();
    }
    return 0;
}