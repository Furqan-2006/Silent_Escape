#include "level.hpp"
#include "mapLoader.hpp"
#include "metaDataLoader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

Level::Level(const std::string &mapPath, float tileSize, sf::RenderWindow &win) : tileSize(tileSize), player(1.5f), window(win)
{
    obstacles = loadMap(mapPath, tileSize);
    auto gridData = loadGridMap(mapPath);
    pathfinder = std::make_unique<PathFinder>(gridData.size(), gridData[0].size(), tileSize, gridData);

    std::string metaPath = mapPath.substr(0, mapPath.find_last_of('.')) + ".json";
    LevelMetadata meta = loadMetadata(metaPath, tileSize);

    player.setPosition(meta.playerPos);

    for (const auto &g : meta.guards)
    {
        addGuard(g.position, g.direction);
    }

    std::cout << "Level loaded\n";
}

void Level::handleInput(const sf::Event::KeyPressed &key)
{
    if (key.scancode == sf::Keyboard::Scancode::E)
    {
        InteractionManager::handle(player, obstacles);
    }
}

void Level::addGuard(const sf::Vector2f &position, const sf::Vector2f &direction)
{
    Guard guard;
    guard.setPosition(position);
    guard.setVelocity(direction);
    Level::guards.push_back(guard);
}

void Level::update(GameState &gameState)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
    {
        player.moveUp(obstacles);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
    {
        player.moveLeft(obstacles);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
    {
        player.moveDown(obstacles);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
    {
        player.moveRight(obstacles);
    }

    for (auto &guard : guards)
    {
        guard.update(player, player.getPosition(), obstacles, gameState, *pathfinder, window, tileSize);
    }
}

void Level::render()
{
    for (auto &ob : obstacles)
        ob.draw(window);

    for (auto &guard : guards)
    {
        guard.drawSightCone(window);
        guard.draw(window);
    }

    player.draw(window);
}