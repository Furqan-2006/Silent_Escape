#include <iostream>
#include <SFML/Window.hpp>

int main()
{
    sf::Window window(sf::VideoMode({1280, 720}), "Silent Escape");
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() && event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape))
            {
                window.close();
            }
            
        }
    }
}