#include "level.hpp"
#include "mapLoader.hpp"
#include "metaDataLoader.hpp"
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

Level::Level(const std::string &mapPath, TextureManager &textureManager, float tileSize, sf::RenderWindow &win)
    : tileSize(tileSize), player(50.f), window(win)
{
    if (!FloorTexture.loadFromFile("../assets/textures/Sprites/floor/floor-3.png"))
    {
        std::cerr << "[ERROR] Cannot load floor textures" << std::endl;
        return;
    }

    mapView.setSize({1366, 720});
    // Load tilemap and obstacles
    obstacles = loadMap(mapPath, textureManager, tileSize);

    // Load pathfinding grid
    auto gridData = loadGridMap(mapPath);
    pathfinder = std::make_unique<PathFinder>(stepSize, gridData.size(), gridData[0].size(), gridData, tileSize);

    int cols = gridData.size();
    int rows = gridData[0].size(); // assuming at least 1 row exists

    for (int y = 1; y < rows; y++)
    {
        for (int x = 1; x < cols; x++)
        {
            sf::Sprite floor(FloorTexture);
            // std::cout << "[LOG] FLoor sprite created" << std::endl;

            floor.setOrigin({FloorTexture.getSize().x / 2.f, FloorTexture.getSize().y - tileSize / 2.f});

            sf::Vector2f isoPos = toIsometric({x, y}, tileSize);
            // isoPos += {100, 388};
            floor.setPosition({std::round(isoPos.x), std::round(isoPos.y)});
            floorSprites.push_back(floor);
        }
    }
    mapHeight = gridData.size();
    mapWidth = gridData[0].size();

    // Load metadata (.json)
    std::string metaPath = mapPath.substr(0, mapPath.find_last_of('.')) + ".json";
    meta = loadMetadata(metaPath, tileSize);

    // Initialize player
    player.setPos(meta.playerPos);
    player.setGridPos(meta.playerGridPos);
    sf::Vector2f isoPos = toIsometric(player.getGridPosition(), 64.f);
    player.setPos(isoPos);

    // Initialize guards from metadata
    for (const auto &g : meta.guards)
    {
        addGuard(g.position, g.gridPos, g.direction, g.patrolPath);
    }

    std::cout << "Level loaded\n";
    std::cout << "[Level] Constructed with tileSize: " << tileSize << std::endl;
}

void Level::reset()
{
    // Reset player position
    player.setPos(meta.playerPos);

    // Reset each guard
    for (std::size_t i = 0; i < meta.guards.size() && i < guards.size(); ++i)
    {
        guards[i].setPos(meta.guards[i].position);
        guards[i].resetState();
    }
    mapView.setCenter(meta.playerPos);
}

void Level::handleInput(const sf::Event::KeyPressed &key)
{
    if (key.scancode == sf::Keyboard::Scancode::E)
    {
        InteractionManager::handle(player, obstacles);
    }
    else if (key.scancode == sf::Keyboard::Scancode::Escape)
    {
        pause();
    }
}

void Level::addGuard(const sf::Vector2f &position, const sf::Vector2i &gridPosition, const sf::Vector2f &direction, const std::vector<sf::Vector2f> &patrolPath)
{
    Guard guard;
    guard.setPos(position);
    sf::Vector2f isoPos = toIsometric(gridPosition, 64.f);
    isoPos += {64.f, 64.f};
    guard.setPos(isoPos);
    guard.setGridPos(gridPosition);
    guard.setVelocity(direction);
    guard.setPatrolPath(patrolPath);

    guards.push_back(guard);
}

void updateCamera(sf::View &view, const sf::Vector2f &playerPos, float mapWidth, float mapHeight, float deltaTime)
{
    sf::Vector2f viewSize = view.getSize();
    float halfWidth = viewSize.x / 2.f;
    float halfHeight = viewSize.y / 2.f;

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

    // Smooth interpolation (lerp)
    float alpha = 1.f * deltaTime;
    sf::Vector2f smoothCenter = currentCenter + alpha * (desiredCenter - currentCenter);
    view.setCenter(smoothCenter);
}

void Level::update(GameState &gameState, float &deltaTime, sf::Clock &gameOverClock)
{
    if (isPaused)
    {
        return;
    }
    updateCamera(mapView, player.getPos(), getMapSize().x, getMapSize().y, deltaTime);

    // Handle player movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        player.moveUp(deltaTime, obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        player.moveLeft(deltaTime, obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        player.moveDown(deltaTime, obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        player.moveRight(deltaTime, obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
        player.moveUp(deltaTime * 10.f, obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
        player.moveLeft(deltaTime * 10.f, obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
        player.moveDown(deltaTime * 10.f, obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
        player.moveRight(deltaTime * 10.f, obstacles);

    // Update all guards
    for (auto &guard : guards)
    {
        guard.update(player, player.getPos(), obstacles, gameState, *pathfinder, window, tileSize, deltaTime, gameOverClock);
    }
}

void Level::render(sf::RenderWindow &window)
{
    window.setView(mapView);

    // draw floor
    for (const auto &tile : floorSprites)
        window.draw(tile);

    // Draw map and entities
    for (auto &ob : obstacles)
    {
        ob.draw(window);
    }
    for (auto &guard : guards)
    {
        guard.drawSightCone(window);
        guard.draw(window);
        // guard.drawPath(window);
    }

    player.draw(window);
    player.drawTileHighlight(window, 64, 32); // assuming 64x32 tile size

    if (isPaused)
    {
        drawPauseOverlay(window);
    }
}

sf::Vector2f Level::getPlayerPos() const
{
    return player.getPos();
}

sf::Vector2f Level::getMapSize() const
{
    return {mapWidth * tileSize, mapHeight * tileSize};
}
void Level::pause()
{
    isPaused = !isPaused;
}

void Level::drawPauseOverlay(sf::RenderWindow &window)
{
    sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);

    sf::Font font;
    if (!font.openFromFile("../assets/Fonts/CURLZ___.TTF"))
    {
        std::cerr << "[Error] Failed to load Font!\n";
        return;
    }

    sf::Text pauseText(font, "Game Paused", 28);
    pauseText.setFillColor(sf::Color::Green);
    pauseText.setOrigin({mapView.getCenter().x, mapView.getCenter().y});

    window.draw(pauseText);
}
