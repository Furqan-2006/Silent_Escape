#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include "mainMenu.hpp"
#include "player.hpp"
#include "gameObj.hpp"

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
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Silent Escape");
    GameState gamestate = GameState::MENU;

    Menu menu(window.getSize().x, window.getSize().y);
    Player player;

    std::vector<GameObject> obstacles;

    obstacles.emplace_back(30.f, 5, sf::Vector2f(200, 150), sf::Color::Red);    // Wall
    obstacles.emplace_back(15.f, 4, sf::Vector2f(300, 300), sf::Color::Yellow); // Box
    obstacles.emplace_back(20.f, 3, sf::Vector2f(400, 100), sf::Color::Blue);   // Door

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
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::W)
                    {
                        player.moveUp(obstacles);
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::A)
                    {
                        player.moveLeft(obstacles);
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::S)
                    {
                        player.moveDown(obstacles);
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::D)
                    {
                        player.moveRight(obstacles);
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::H)
                    {
                        player.hack();
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::G)
                    {
                        player.hide();
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::E)
                    {
                        player.disguise();
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
            // window.draw(levelText);
            for (auto &ob : obstacles)
            {
                ob.draw(window);
            }
            player.draw(window);
            break;
        case GameState::EXIT:
            window.close();
            break;
        }

        window.display();
    }
    return 0;
}