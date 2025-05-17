#include "levelmanager.hpp"

LevelManager::LevelManager(sf::RenderWindow &window, float tileSize)
    : nextReady(false), stopThread(false), window(window), tileSize(tileSize) {}

LevelManager::~LevelManager()
{
    stopThread = true;
    if (preloadThread.joinable())
    {
        preloadThread.join();
    }
}

void LevelManager::loadInitialLevel(const std::string &filename, TextureManager &textureManager)
{
    currentLevel = std::make_shared<Level>(filename, textureManager, tileSize, window);
}

void LevelManager::preloadNextLevel(const std::string &filename, TextureManager &textureManager)
{
    nextReady = false;

    if (preloadThread.joinable())
    {
        preloadThread.join();
    }

    preloadThread = std::thread([this, filename, textureManagerPtr = &textureManager]()
                                {
    if (stopThread.load()) return;

    auto loadedLevel = std::make_shared<Level>(filename, *textureManagerPtr, tileSize, window);
    if (!stopThread.load())
    {
        nextLevel = loadedLevel;
        nextReady = true;
    } });
}

bool LevelManager::isNextReady() const
{
    return nextReady.load();
}

void LevelManager::switchToNextLevel()
{
    if (!nextReady.load())
        return;

    currentLevel = nextLevel;
    nextLevel.reset();
    nextReady = false;
}

std::shared_ptr<Level> LevelManager::getCurrentLevel()
{
    return currentLevel;
}

LevelManager::LevelManager(LevelManager &&other) noexcept
    : currentLevel(std::move(other.currentLevel)),
      nextLevel(std::move(other.nextLevel)),
      nextReady(other.nextReady.load()),
      stopThread(false),
      window(other.window),
      tileSize(other.tileSize)
{
    if (other.preloadThread.joinable())
    {
        other.preloadThread.join();
    }
    preloadThread = std::move(other.preloadThread);
}

LevelManager &LevelManager::operator=(LevelManager &&other) noexcept
{
    if (this != &other)
    {
        stopThread = true;
        if (preloadThread.joinable())
        {
            preloadThread.join();
        }

        currentLevel = std::move(other.currentLevel);
        nextLevel = std::move(other.nextLevel);
        nextReady = other.nextReady.load();
        stopThread = false;

        preloadThread = std::move(other.preloadThread);
    }
    return *this;
}

void LevelManager::reset()
{
    stopThread = true;
    if (preloadThread.joinable())
    {
        preloadThread.join();
    }
    stopThread = false;
    currentLevel.reset();
    nextLevel.reset();
    nextReady = false;
}
