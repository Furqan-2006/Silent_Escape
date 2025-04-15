#include <iostream>
#include <SFML/Graphics.hpp>
#include "mainMenu.hpp"

int main()
{
    sf::RenderWindow window(sf::VideoMode({900, 640}), "Game Menu");
    Menu menu(window.getSize().x, window.getSize().y);

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
                    int selected = menu.getSelectedIndex();
                    if (selected == 0)
                    {
                        std::cout << "Game start\n";
                    }
                    else if (selected == 1)
                    {
                        std::cout << "Options\n";
                    }
                    else if (selected == 2)
                    {
                        window.close();
                    }
                }
            }
        }
        window.clear();
        menu.draw(window);
        window.display();
    }
    return 0;
}