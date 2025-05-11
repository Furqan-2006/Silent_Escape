#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include "level.hpp"
#include "texturemanager.hpp"

class LevelManager
{
public:
    LevelManager(sf::RenderWindow &window, float tileSize);
    ~LevelManager();

    void loadInitialLevel(const std::string &filename, TextureManager &textureManager);
    void preloadNextLevel(const std::string &filename, TextureManager &textureManager);
    bool isNextReady() const;
    void switchToNextLevel();
    std::shared_ptr<Level> getCurrentLevel();
    void reset();

    LevelManager(LevelManager &&other) noexcept;
    LevelManager &operator=(LevelManager &&other) noexcept;

private:
    std::shared_ptr<Level> currentLevel;
    std::shared_ptr<Level> nextLevel;
    std::atomic<bool> nextReady;
    std::atomic<bool> stopThread;
    std::thread preloadThread;
    sf::RenderWindow &window;
    float tileSize;
};
