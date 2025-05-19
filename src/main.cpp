#include "main.hpp"

// Create a window with a size of 1280x720
sf::RenderWindow window(sf::VideoMode({1280, 720}), "Silent Escape");

// Initialize various game states and menus
GameState gameState = GameState::MENU;                       // The game starts in the MENU state
Menu menu(window.getSize().x, window.getSize().y);           // Main menu
LevelMenu levelmenu(window.getSize().x, window.getSize().y); // Level selection menu

// Declare a clock to track the game over state duration
sf::Clock gameOverClock;

// Set the tile size used in the game
float tileSize = 64.f;

// Level manager for managing level transitions
LevelManager levelManager(window, tileSize);

// Texture manager to handle loading and accessing textures
TextureManager textureManager;

// Handle menu input (key presses) when in the MENU state
void handleMenuInput(const sf::Event::KeyPressed &key, Menu &menu, GameState &gameState, sf::RenderWindow &window)
{
    if (key.scancode == sf::Keyboard::Scancode::Up) // Move the selection up in the menu
        menu.moveUp();
    else if (key.scancode == sf::Keyboard::Scancode::Down) // Move the selection down in the menu
        menu.moveDown();
    else if (key.scancode == sf::Keyboard::Scancode::Enter) // Enter key pressed
    {
        switch (menu.getSelectedIndex()) // Check the selected menu option
        {
        case 0: // Start the level selection menu
            gameState = GameState::LEVEL_MENU;
            break;
        case 1: // Options menu (coming soon)
            std::cout << "[LOG] Options Menu Selected (coming soon)\n";
            break;
        case 2: // Exit the game
            gameState = GameState::EXIT;
            window.close();
            break;
        }
    }
}

// Handle level menu input (key presses) when in the LEVEL_MENU state
void handleLevelMenuInput(const sf::Event::KeyPressed &key, LevelMenu &levelmenu, GameState &gameState, sf::RenderWindow &window)
{
    if (key.scancode == sf::Keyboard::Scancode::Right) // Move selection right
        levelmenu.moveRight();
    else if (key.scancode == sf::Keyboard::Scancode::Left) // Move selection left
        levelmenu.moveLeft();
    else if (key.scancode == sf::Keyboard::Scancode::Enter) // Enter key pressed
    {
        // Load the selected level based on the index and set the game state accordingly
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
        case 4:
            levelManager.loadInitialLevel("../assets/maps/level5.txt", textureManager);
            gameState = GameState::LEVEL_3;
            break;
        }
    }
    else if (key.scancode == sf::Keyboard::Scancode::Escape) // Escape key pressed (go back to MENU)
    {
        gameState = GameState::MENU;
    }
}

// Handle game over input (key press) when in the GAME_OVER state
void handleGameOverInput(const sf::Event::KeyPressed &key)
{
    if (key.scancode == sf::Keyboard::Scancode::Escape) // Escape key pressed (go back to MENU)
    {
        gameState = GameState::MENU;
    }
}

// Update the game state when the game is over (reset and transition to MENU after 2 seconds)
void updateGameOverState(GameState &gameState, sf::Clock &clock)
{
    levelManager.reset();                         // Reset the level manager (stop threads, reset levels)
    if (clock.getElapsedTime().asSeconds() > 2.f) // After 2 seconds, go back to the MENU state
    {
        gameState = GameState::MENU;
    }
}

// Render the main menu
void renderMenu(sf::RenderWindow &window, Menu &menu)
{
    menu.draw(window); // Draw the main menu
}

// Render the level selection menu
void renderLevelMenu(sf::RenderWindow &window, LevelMenu &levelmenu)
{
    levelmenu.draw(window); // Draw the level selection menu
}

int main()
{
    window.setFramerateLimit(60); // Set the frame rate limit to 60 FPS

    sf::Clock deltaClock; // Delta clock to track time per frame
    // Game Over Text: Initialization
    sf::Font font;
    if (!font.openFromFile("../assets/Fonts/CURLZ___.TTF")) // Load the font for "Game Over"
    {
        std::cerr << "Error loading font\n";
    }
    sf::Text levelText(font); // Text for the "Game Over" message
    levelText.setCharacterSize(32);
    levelText.setFillColor(sf::Color::White);
    levelText.setString("Game Over!!");
    levelText.setPosition({100, 250});
    // Game Over Text: End

    // Load textures for different game objects
    textureManager.load("wall-LR", "../assets/textures/Sprites/wall/Sprite-0002.png");
    textureManager.load("wall-TB", "../assets/textures/Sprites/wall/Sprite-0003.png");
    textureManager.load("player-L", "../assets/textures/Sprites/player/player-1.png");
    textureManager.load("guard", "../assets/textures/Sprites/guard/guard.png");

    // Game loop: runs until the window is closed
    while (window.isOpen())
    {
        float deltaTime = deltaClock.restart().asSeconds(); // Track time for this frame
        sf::Event::KeyPressed currentKeyPressed;
        bool keyPressedThisFrame = false;

        while (const std::optional event = window.pollEvent()) // Poll events (keyboard, mouse, etc.)
        {
            if (event->is<sf::Event::Closed>()) // If the window is closed
                window.close();

            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                currentKeyPressed = *keyPressed; // Store the key pressed
                keyPressedThisFrame = true;      // Mark that a key was pressed

                // Handle key input based on the current game state
                if (gameState == GameState::MENU)
                    handleMenuInput(*keyPressed, menu, gameState, window);
                else if (gameState == GameState::LEVEL_MENU)
                    handleLevelMenuInput(*keyPressed, levelmenu, gameState, window);
                else if (gameState == GameState::GAME_OVER)
                    handleGameOverInput(*keyPressed);
            }
            else if (const auto *mouseClicked = event->getIf<sf::Event::MouseButtonPressed>())
            {
                // Handle mouse clicks in the LEVEL_MENU state
                if (gameState == GameState::LEVEL_MENU)
                {
                    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);    // Get mouse position
                    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos); // Convert to world coordinates
                    levelmenu.onClick(worldPos);                               // Select a level if clicked
                }
            }
        }

        window.clear(); // Clear the window to prepare for new frame

        auto level = levelManager.getCurrentLevel(); // Get the current level from the LevelManager
        if (level && (gameState == GameState::LEVEL_1 || gameState == GameState::LEVEL_2 || gameState == GameState::LEVEL_3))
        {
            if (keyPressedThisFrame)
                level->handleInput(currentKeyPressed); // Handle input for the current level

            level->update(gameState, deltaTime, gameOverClock); // Update the level's state
            level->render(window);                              // Render the current level
        }

        window.setView(window.getDefaultView()); // Reset the view to the default view

        // Render the appropriate menu or game state based on the current state
        switch (gameState)
        {
        case GameState::MENU:
            renderMenu(window, menu); // Render the main menu
            break;
        case GameState::LEVEL_MENU:
            renderLevelMenu(window, levelmenu); // Render the level selection menu
            break;
        case GameState::LEVEL_CLEAR:
            gameState = GameState::LEVEL_MENU; // Transition back to the level menu (commented out code)
            break;
        case GameState::GAME_OVER:
        {
            window.draw(levelText);                        // Draw the "Game Over" text
            updateGameOverState(gameState, gameOverClock); // Update game over state (wait for 2 seconds)
            break;
        }
        case GameState::EXIT:
            window.close(); // Exit the game
            break;
        }

        window.display(); // Display the rendered frame
    }
    return 0; // Exit the program
}
