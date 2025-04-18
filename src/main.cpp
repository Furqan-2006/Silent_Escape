#include "main.hpp"

sf::RenderWindow window(sf::VideoMode({800, 600}), "Silent Escape");
GameState gameState = GameState::MENU;

Menu menu(window.getSize().x, window.getSize().y);
Player player(0.2);
Guard guard;
sf::Clock gameOverClock;
std::vector<GameObject> obstacles = loadMap("../assets/maps/level1.txt", 15.f);

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

void handleLevel1Input(const sf::Event::KeyPressed &key, Player &player, std::vector<GameObject> &obstacles)
{
    switch (key.scancode)
    {
    case sf::Keyboard::Scancode::E:
        InteractionManager::handle(player, obstacles);
        break;
    case sf::Keyboard::Scancode::Escape:
        std::cout << "[LOG] Escape pressed\n";
        break;
    default:
        break;
    }
}

void updateGameOverState(GameState &gameState, sf::Clock &clock)
{
    if (clock.getElapsedTime().asSeconds() > 2.f)
        gameState = GameState::MENU;
} // GameoverText:start

void renderMenu(sf::RenderWindow &window, Menu &menu)
{
    menu.draw(window);
}

void renderLevel1(sf::RenderWindow &window, Player &player, Guard &guard, std::vector<GameObject> &obstacles, GameState &gameState)
{
    for (auto &ob : obstacles)
        ob.draw(window);

    guard.update(player, player.getPosition(), obstacles, gameState);
    player.draw(window);
    guard.drawSightCone(window);
    guard.draw(window);
}

int main()
{
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
                    handleLevel1Input(*keyPressed, player, obstacles);
            }
        }

        window.clear();

        switch (gameState)
        {
        case GameState::MENU:
            renderMenu(window, menu);
            break;
        case GameState::LEVEL_1:
            renderLevel1(window, player, guard, obstacles, gameState);
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
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
                player.moveUp(obstacles);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
                player.moveLeft(obstacles);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
                player.moveDown(obstacles);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
                player.moveRight(obstacles);
        }
    }
    return 0;
}