#include "main.hpp"

sf::RenderWindow window(sf::VideoMode({800, 600}), "Silent Escape");
GameState gameState = GameState::MENU;
Menu menu(window.getSize().x, window.getSize().y);
LevelMenu levelmenu(window.getSize().x, window.getSize().y);

sf::Clock gameOverClock;
float tileSize = 40.f;
Level level1("../assets/maps/level1.txt", tileSize, window);

Level level2("../assets/maps/level2.txt", tileSize, window);

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
            gameState = GameState::LEVEL_MENU;
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

void handleLevelMenuInput(const sf::Event::KeyPressed &key, LevelMenu &levelmenu, GameState &gameState, sf::RenderWindow &window)
{
    if (key.scancode == sf::Keyboard::Scancode::Right)
        levelmenu.moveRight();
    else if (key.scancode == sf::Keyboard::Scancode::Left)
        levelmenu.moveLeft();
    else if (key.scancode == sf::Keyboard::Scancode::Enter)
    {
        switch (levelmenu.getSelectedLevel())
        {
        case 0:
            gameState = GameState::LEVEL_1;
            break;
        case 1:
            gameState = GameState::LEVEL_2;
            break;
        }
    }
    else if (key.scancode == sf::Keyboard::Scancode::Escape)
    {
        gameState = GameState::MENU;
    }
}

void updateGameOverState(GameState &gameState, sf::Clock &clock)
{

    if (clock.getElapsedTime().asSeconds() > 5.f)
        gameState = GameState::MENU;
}

void renderMenu(sf::RenderWindow &window, Menu &menu)
{
    menu.draw(window);
}
void renderLevelMenu(sf::RenderWindow &window, LevelMenu &levelmenu)
{
    levelmenu.draw(window);
}

int main()
{
    window.setFramerateLimit(60);

    sf::Clock deltaClock;
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
        float deltaTime = deltaClock.restart().asSeconds();

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (gameState == GameState::MENU)
                    handleMenuInput(*keyPressed, menu, gameState, window);
                else if (gameState == GameState::LEVEL_MENU)
                    handleLevelMenuInput(*keyPressed, levelmenu, gameState, window);
                else if (gameState == GameState::LEVEL_1)
                    level1.handleInput(*keyPressed);
                else if (gameState == GameState::LEVEL_2)
                    level2.handleInput(*keyPressed);
            }
            else if (const auto *mouseClicked = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (gameState == GameState::LEVEL_MENU)
                {
                    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
                    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);
                    levelmenu.onClick(worldPos);
                }
            }
        }

        window.clear();

        switch (gameState)
        {
        case GameState::MENU:
            renderMenu(window, menu);
            break;
        case GameState::LEVEL_MENU:
            renderLevelMenu(window, levelmenu);
            break;
        case GameState::LEVEL_1:
            level1.render();
            break;
        case GameState::LEVEL_2:
            level2.render();
            break;
        case GameState::GAME_OVER:
            window.draw(levelText);
            gameOverClock.restart();
            updateGameOverState(gameState, gameOverClock);
            break;
        case GameState::EXIT:
            window.close();
            break;
        }

        window.display();
        if (gameState == GameState::LEVEL_1)
        {
            level1.update(gameState, deltaTime);
        }
        else if (gameState == GameState::LEVEL_2)
        {
            level2.update(gameState, deltaTime);
        }
    }
    return 0;
}