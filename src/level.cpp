#include "level.hpp"
#include "mapLoader.hpp"
#include "metaDataLoader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

Level::Level(const std::string &mapPath, float tileSize, sf::RenderWindow &win)
    : tileSize(tileSize), player(1.5f), window(win)
{
    mapView.setSize({800, 600});
    // Load tilemap and obstacles
    obstacles = loadMap(mapPath, tileSize);

    // Load pathfinding grid
    auto gridData = loadGridMap(mapPath);
    pathfinder = std::make_unique<PathFinder>(stepSize, gridData.size(), gridData[0].size(), gridData, tileSize);

    mapHeight = gridData.size();
    mapWidth = gridData[0].size();

    // Load metadata (.json)
    std::string metaPath = mapPath.substr(0, mapPath.find_last_of('.')) + ".json";
    meta = loadMetadata(metaPath, tileSize);

    // Initialize player
    player.setPosition(meta.playerPos);

    // Initialize guards from metadata
    for (const auto &g : meta.guards)
    {
        addGuard(g.position, g.direction, g.patrolPath);
    }

    std::cout << "Level loaded\n";
}

void Level::reset()
{
    // Reset player position
    player.setPosition(meta.playerPos);

    // Reset each guard
    for (std::size_t i = 0; i < meta.guards.size() && i < guards.size(); ++i)
    {
        guards[i].setPosition(meta.guards[i].position);
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

void Level::addGuard(const sf::Vector2f &position, const sf::Vector2f &direction, const std::vector<sf::Vector2f> &patrolPath)
{
    Guard guard;
    guard.setPosition(position);
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
                         : mapWidth / 2.f;

    float clampedY = (mapHeight > viewSize.y)
                         ? std::clamp(playerPos.y, halfHeight, mapHeight - halfHeight)
                         : mapHeight / 2.f;

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
    updateCamera(mapView, player.getPosition(), getMapSize().x, getMapSize().y, deltaTime);

    // Handle player movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        player.moveUp(obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        player.moveLeft(obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        player.moveDown(obstacles);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        player.moveRight(obstacles);

    // Update all guards
    for (auto &guard : guards)
    {
        guard.update(player, player.getPosition(), obstacles, gameState, *pathfinder, window, tileSize, deltaTime, gameOverClock);
    }
}

void Level::render(sf::RenderWindow &window)
{
    window.setView(mapView);

    // Draw map and entities
    for (auto &ob : obstacles)
        ob.draw(window);
    for (auto &guard : guards)
    {
        guard.drawSightCone(window);
        guard.draw(window);
        // guard.drawPath(window);
    }
    player.draw(window);
    if (isPaused)
    {
        drawPauseOverlay(window);
    }
}

sf::Vector2f Level::getPlayerPos() const
{
    return player.getPosition();
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
