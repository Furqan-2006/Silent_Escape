#include "main.hpp"

sf::RenderWindow window(sf::VideoMode({800, 600}), "Silent Escape");
GameState gameState = GameState::MENU;
Menu menu(window.getSize().x, window.getSize().y);

sf::Clock gameOverClock;
float tileSize = 40.f;
Level level1("../assets/maps/level1.txt", tileSize, window);

void handleMenuInput(const sf::Event::KeyPressed &key, Menu &menu, GameState &gameState, sf::RenderWindow &window)
{
    if (key.scancode == sf::Keyboard::Scancode::Up)
        menu.moveUp();
    else if (key.scancode == sf::Keyboard::Scancode::Down)
        menu.moveDown();
    else if (key.scancode == sf::Keyboard::Scancode::Enter)
    {
        switch (menu.getSelectedIndex())
        {
        case 0:
            gameState = GameState::LEVEL_1;
            break;
        case 1:
            std::cout << "[LOG] Options Menu Selected (coming soon)\n";
            break;
        case 2:
            gameState = GameState::EXIT;
            window.close();
            break;
        }
    }
}

void updateGameOverState(GameState &gameState, sf::Clock &clock)
{
    if (clock.getElapsedTime().asSeconds() > 2.f)
        gameState = GameState::MENU;
}

void renderMenu(sf::RenderWindow &window, Menu &menu)
{
    menu.draw(window);
}

int main()
{
    window.setFramerateLimit(60);
    // GameoverText:start
    sf::Font font;
    if (!font.openFromFile("../assets/Fonts/CURLZ___.TTF"))
    {
        std::cerr << "Error loading font\n";
    }
    sf::Text levelText(font);
    levelText.setCharacterSize(32);
    levelText.setFillColor(sf::Color::White);
    levelText.setString("Game Over!!");
    levelText.setPosition({100, 250});
    // GameoverText:end

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (gameState == GameState::MENU)
                    handleMenuInput(*keyPressed, menu, gameState, window);
                else if (gameState == GameState::LEVEL_1)
                    level1.handleInput(*keyPressed);
            }
        }

        window.clear();

        switch (gameState)
        {
        case GameState::MENU:
            renderMenu(window, menu);
            break;
        case GameState::LEVEL_1:
            level1.render();
            break;
        case GameState::GAME_OVER:
            window.draw(levelText);
            updateGameOverState(gameState, gameOverClock);
            break;
        case GameState::EXIT:
            window.close();
            break;
        }

        window.display();
        if (gameState == GameState::LEVEL_1)
        {
            level1.update(gameState);
        }
    }
    return 0;
}