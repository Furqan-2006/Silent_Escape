#pragma once

#include <thread>
#include <memory>
#include <atomic>
#include "level.hpp"

class LevelManager
{
    sf::RenderWindow &window;
    float tileSize = 40.f;

    std::shared_ptr<Level> currentLevel;
    std::shared_ptr<Level> nextLevel;
    std::thread preloadThread;
    std::atomic<bool> nextReady;
    std::atomic<bool> stopThread;

public:
    LevelManager(sf::RenderWindow &window, float tileSize);

    ~LevelManager();
    LevelManager(const LevelManager &) = delete;
    LevelManager &operator=(const LevelManager &) = delete;
    LevelManager(LevelManager &&) noexcept;
    LevelManager &operator=(LevelManager &&) noexcept;

    void loadInitialLevel(const std::string &filename);
    void preloadNextLevel(const std::string &filename);
    void switchToNextLevel();
    void reset();

    std::shared_ptr<Level> getCurrentLevel();
    bool isNextReady() const;
};