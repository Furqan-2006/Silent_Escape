#include "level.hpp"
#include "mapLoader.hpp"
#include "metaDataLoader.hpp"
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

// Constructor to initialize the level with map path, texture manager, tile size, and the window
Level::Level(const std::string &mapPath, TextureManager &textureManager, float tileSize, sf::RenderWindow &win)
    : tileSize(tileSize), player(textureManager.get("player-L"), 3.f), window(win)
{
    // Load the floor texture and check for errors
    if (!FloorTexture.loadFromFile("../assets/textures/Sprites/floor/floor-3.png"))
    {
        std::cerr << "[ERROR] Cannot load floor textures" << std::endl;
        return;
    }

    // Set up the map view size
    mapView.setSize({1366, 720});

    // Load the obstacles in the map
    obstacles = loadMap(mapPath, textureManager, tileSize);

    // Load pathfinding grid data
    auto gridData = loadGridMap(mapPath);
    pathfinder = std::make_unique<PathFinder>(stepSize, gridData.size(), gridData[0].size(), gridData, tileSize);

    int cols = gridData.size();
    int rows = gridData[0].size();  // assuming at least 1 row exists

    // Create floor sprites for each tile in the grid
    for (int y = 1; y < rows; y++)
    {
        for (int x = 1; x < cols; x++)
        {
            sf::Sprite floor(FloorTexture);

            floor.setOrigin({FloorTexture.getSize().x / 2.f, FloorTexture.getSize().y - tileSize / 2.f});

            // Convert grid position to isometric position
            sf::Vector2f isoPos = toIsometric({x, y}, tileSize);
            floor.setPosition({std::round(isoPos.x), std::round(isoPos.y)});
            floorSprites.push_back(floor); // Add floor sprite to the collection
        }
    }

    mapHeight = gridData.size();
    mapWidth = gridData[0].size();

    // Load metadata from JSON file
    std::string metaPath = mapPath.substr(0, mapPath.find_last_of('.')) + ".json";
    meta = loadMetadata(metaPath, tileSize);

    // Initialize player position and grid position
    player.setPos(meta.playerPos);
    player.setGridPos(meta.playerGridPos);
    sf::Vector2f isoPos = toIsometric(player.getGridPosition(), 64.f);
    player.setPos(isoPos);

    // Initialize guards from metadata
    for (const auto &g : meta.guards)
    {
        addGuard(g.position, g.gridPos, g.direction, g.patrolPath, textureManager.get("guard"));
    }

    sf::Vector2i Goal = meta.goalPos; // Goal position (not used in this constructor)

    std::cout << "Level loaded\n";
    std::cout << "[Level] Constructed with tileSize: " << tileSize << std::endl;
}

// Function to reset the level (e.g., after player dies or restarts)
void Level::reset()
{
    // Reset player position to initial position from metadata
    player.setPos(meta.playerPos);

    // Reset each guard's position and state
    for (std::size_t i = 0; i < meta.guards.size() && i < guards.size(); ++i)
    {
        guards[i].setPos(meta.guards[i].position);
        guards[i].resetState();  // Reset guard state
    }

    mapView.setCenter(meta.playerPos);  // Reset camera center to player's position
}

// Function to handle input events (key presses)
void Level::handleInput(const sf::Event::KeyPressed &key)
{
    if (key.scancode == sf::Keyboard::Scancode::E)
    {
        InteractionManager::handle(player, obstacles);  // Handle player interaction
    }
    else if (key.scancode == sf::Keyboard::Scancode::Escape)
    {
        pause();  // Pause the game
    }
}

// Function to add a guard to the level
void Level::addGuard(const sf::Vector2f &position, const sf::Vector2i &gridPosition, const sf::Vector2f &direction, const std::vector<sf::Vector2f> &patrolPathGrid, const sf::Texture &tex)
{
    Guard guard(tex);

    // Convert guard position from grid coordinates to isometric screen position
    sf::Vector2f isoPos = toIsometric(gridPosition, tileSize);
    isoPos += sf::Vector2f(tileSize * 0.5, tileSize * 0.5);  // Offset to center the guard on the tile
    guard.setPos(isoPos);

    guard.setVelocity(direction);  // Set the guard's velocity (direction)

    guard.setPatrolPath(patrolPathGrid);  // Set the patrol path for the guard

    guards.push_back(std::move(guard));  // Add the guard to the guards list
}

// Function to update the camera position based on the player's position
void updateCamera(sf::View &view, const sf::Vector2f &playerPos, float mapWidth, float mapHeight, float deltaTime)
{
    sf::Vector2f viewSize = view.getSize();
    float halfWidth = viewSize.x / 2.f;
    float halfHeight = viewSize.y / 2.f;

    // Clamp camera position to ensure it stays within map bounds
    float clampedX = (mapWidth > viewSize.x)
                         ? std::clamp(playerPos.x, halfWidth, mapWidth - halfWidth)
                         : (mapWidth != 0 ? mapWidth / 2.f : 0.f);

    float clampedY = (mapHeight > viewSize.y)
                         ? std::clamp(playerPos.y, halfHeight, mapHeight - halfHeight)
                         : (mapHeight != 0 ? mapHeight / 2.f : 0.f);

    if (mapWidth == 0 || mapHeight == 0)
    {
        std::cerr << "[Error] Division by zero detected in map dimensions.\n";
        return;
    }

    sf::Vector2f desiredCenter(clampedX, clampedY);
    sf::Vector2f currentCenter = view.getCenter();

    // Smooth interpolation for camera movement (lerp)
    float alpha = 1.f * deltaTime;
    sf::Vector2f smoothCenter = currentCenter + alpha * (desiredCenter - currentCenter);
    view.setCenter(smoothCenter);  // Update camera center
}

// Function to update the game level (player and guards)
void Level::update(GameState &gameState, float &deltaTime, sf::Clock &gameOverClock)
{
    if (isPaused)
    {
        return;  // Do nothing if the game is paused
    }

    updateCamera(mapView, player.getPos(), getMapSize().x, getMapSize().y, deltaTime);  // Update camera position

    // Handle player movement with different keys
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        player.moveUp(deltaTime, obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        player.moveLeft(deltaTime, obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        player.moveDown(deltaTime, obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        player.moveRight(deltaTime, obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
        player.moveUp(deltaTime * 8.f, obstacles);  // Run movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
        player.moveLeft(deltaTime * 8.f, obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
        player.moveDown(deltaTime * 8.f, obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
        player.moveRight(deltaTime * 8.f, obstacles);

    player.update(gameState);  // Update player status

    // Update all guards
    for (auto &guard : guards)
    {
        guard.update(player, player.getPos(), obstacles, gameState, *pathfinder, window, tileSize, deltaTime, gameOverClock);
    }
}

// Function to render the level to the window
void Level::render(sf::RenderWindow &window)
{
    window.setView(mapView);  // Set the view for the window

    // Draw floor tiles
    for (const auto &tile : floorSprites)
        window.draw(tile);

    // Draw obstacles
    for (auto &ob : obstacles)
    {
        ob.draw(window);
    }

    // Draw guards and their sight cone
    for (auto &guard : guards)
    {
        guard.drawSightCone(window);  // Draw the guard's sight cone
        guard.draw(window);  // Draw the guard
    }

    player.draw(window);  // Draw the player
    player.drawTileHighlight(window, 64, 32);  // Draw the tile highlight (64x32 size)

    // Draw pause overlay if game is paused
    if (isPaused)
    {
        drawPauseOverlay(window);
    }
}

// Function to get the player's current position
sf::Vector2f Level::getPlayerPos() const
{
    return player.getPos();  // Return player's position
}

// Function to get the map size
sf::Vector2f Level::getMapSize() const
{
    return {mapWidth * tileSize, mapHeight * tileSize};  // Return the map's width and height
}

// Function to pause or unpause the game
void Level::pause()
{
    isPaused = !isPaused;  // Toggle the paused state
}

// Function to draw the pause overlay when the game is paused
void Level::drawPauseOverlay(sf::RenderWindow &window)
{
    sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));  // Semi-transparent black overlay
    window.draw(overlay);

    // Load font for the pause text
    sf::Font font;
    if (!font.openFromFile("../assets/Fonts/CURLZ___.TTF"))
    {
        std::cerr << "[Error] Failed to load Font!\n";
        return;
    }

    // Set up the pause text
    sf::Text pauseText(font, "Game Paused", 28);
    pauseText.setFillColor(sf::Color::Green);

    // Center the text on the screen
    sf::FloatRect bounds = pauseText.getLocalBounds();
    pauseText.setOrigin(bounds.getCenter());
    pauseText.setPosition({window.getSize().x / 2.f, window.getSize().y / 2.f});

    window.draw(pauseText);  // Draw the pause text
}
