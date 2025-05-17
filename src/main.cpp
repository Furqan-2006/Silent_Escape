#include "main.hpp"

sf::RenderWindow window(sf::VideoMode({1280, 720}), "Silent Escape");
// sf::View view(sf::FloatRect({0, 0}, {800, 600}));

GameState gameState = GameState::MENU;
Menu menu(window.getSize().x, window.getSize().y);
LevelMenu levelmenu(window.getSize().x, window.getSize().y);

sf::Clock gameOverClock;
float tileSize = 64.f;

LevelManager levelManager(window, tileSize);
TextureManager textureManager;

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
            levelManager.loadInitialLevel("../assets/maps/level1.txt", textureManager);
            gameState = GameState::LEVEL_1;
            break;
        case 1:
            levelManager.loadInitialLevel("../assets/maps/level2.txt", textureManager);
            gameState = GameState::LEVEL_2;
            break;
        case 2:
            levelManager.loadInitialLevel("../assets/maps/level3.txt", textureManager);
            gameState = GameState::LEVEL_3;
            break;

        case 3:
            //     levelManager.loadInitialLevel("../assets/maps/level4.txt", textureManager);
            //     gameState = GameState::LEVEL_3;
            //     break;
            // case 4:
            levelManager.loadInitialLevel("../assets/maps/level5.txt", textureManager);
            gameState = GameState::LEVEL_3;
            break;
        }
    }
    else if (key.scancode == sf::Keyboard::Scancode::Escape)
    {
        gameState = GameState::MENU;
    }
}
void handleGameOverInput(const sf::Event::KeyPressed &key)
{

    if (key.scancode == sf::Keyboard::Scancode::Escape)
    {
        gameState = GameState::MENU;
    }
}
void updateGameOverState(GameState &gameState, sf::Clock &clock)
{
    levelManager.reset();
    if (clock.getElapsedTime().asSeconds() > 2.f)
    {
        gameState = GameState::MENU;
    }
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

    // Load Textures
    textureManager.load("wall-LR", "../assets/textures/Sprites/wall/Sprite-0002.png");
    textureManager.load("wall-TB", "../assets/textures/Sprites/wall/Sprite-0003.png");
    textureManager.load("player-L", "../assets/textures/Sprites/player/player-1.png");
    textureManager.load("guard", "../assets/textures/Sprites/guard/guard.png");

    while (window.isOpen())
    {
        float deltaTime = deltaClock.restart().asSeconds();
        sf::Event::KeyPressed currentKeyPressed;
        bool keyPressedThisFrame = false;

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                currentKeyPressed = *keyPressed;
                keyPressedThisFrame = true;
                if (gameState == GameState::MENU)
                    handleMenuInput(*keyPressed, menu, gameState, window);
                else if (gameState == GameState::LEVEL_MENU)
                    handleLevelMenuInput(*keyPressed, levelmenu, gameState, window);
                else if (gameState == GameState::GAME_OVER)
                    handleGameOverInput(*keyPressed);
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
        // window.setView(view);

        auto level = levelManager.getCurrentLevel();
        if (level && (gameState == GameState::LEVEL_1 || gameState == GameState::LEVEL_2 || gameState == GameState::LEVEL_3))
        {
            if (keyPressedThisFrame)
                level->handleInput(currentKeyPressed);

            level->update(gameState, deltaTime, gameOverClock);
            level->render(window);
        }

        window.setView(window.getDefaultView());
        switch (gameState)
        {
        case GameState::MENU:
            renderMenu(window, menu);
            break;
        case GameState::LEVEL_MENU:
            renderLevelMenu(window, levelmenu);
            break;
        case GameState::LEVEL_CLEAR:
            GameState::LEVEL_MENU;
            break;
        case GameState::GAME_OVER:
        {
            window.draw(levelText);
            updateGameOverState(gameState, gameOverClock);
            break;
        }
        case GameState::EXIT:
            window.close();
            break;
        }
        window.display();
    }
    return 0;
}