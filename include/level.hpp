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
#include "metaDataLoader.hpp"
#include "textureManager.hpp"

class Level
{
private:
    sf::RenderWindow &window;
    sf::View mapView;

    float tileSize = 40.f;
    float stepSize = tileSize;

    sf::Texture FloorTexture;
    std::vector<sf::Sprite> floorSprites;

    std::vector<GameObject> obstacles;
    std::vector<Guard> guards;
    Player player;
    std::unique_ptr<PathFinder> pathfinder;

    float mapHeight;
    float mapWidth;

    bool isPaused = false;

    LevelMetadata meta;

    void addGuard(const sf::Vector2f &position, const sf::Vector2i &gridPosition, const sf::Vector2f &direction, const std::vector<sf::Vector2f> &patrolPath);

public:
    Level(const std::string &mapPath, TextureManager &textureManager, float tileSize, sf::RenderWindow &win);

    void handleInput(const sf::Event::KeyPressed &key);
    void update(GameState &gameState, float &deltaTime, sf::Clock &gameOverClock);
    void render(sf::RenderWindow &window);
    void drawPauseOverlay(sf::RenderWindow &window);
    void reset();
    void pause();

    sf::Vector2f getPlayerPos() const;
    sf::Vector2f getMapSize() const;
};
