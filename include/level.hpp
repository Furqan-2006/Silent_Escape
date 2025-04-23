#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "player.hpp"
#include "guard.hpp"
#include "gameObj.hpp"
#include "pathfinder.hpp"
#include "gameStateEnum.hpp"
#include "interactionManager.hpp"

class Level
{
private:
    sf::RenderWindow &window;
    float tileSize;

    std::vector<GameObject> obstacles;
    std::vector<Guard> guards;
    Player player;
    std::unique_ptr<PathFinder> pathfinder;

    void addGuard(const sf::Vector2f &position);

public:
    Level(const std::string &mapPath, float tileSize, sf::RenderWindow &win);

    void handleInput(const sf::Event::KeyPressed &key);
    void update(GameState &gameState);
    void render();
    
    void addGuard(const sf::Vector2f &position, const sf::Vector2f &direction);
};
