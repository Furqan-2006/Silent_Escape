#include "levelmanager.hpp"

LevelManager::LevelManager(sf::RenderWindow &window, float tileSize) : nextReady(false), stopThread(false), window(window), tileSize(tileSize) {}

LevelManager::~LevelManager()
{
    stopThread = true;
    if (preloadThread.joinable())
    {
        preloadThread.join();
    }
}

void LevelManager::loadInitialLevel(const std::string &filename)
{
    currentLevel = std::make_shared<Level>(filename, tileSize, window);
}
void LevelManager::preloadNextLevel(const std::string &filename)
{
    nextReady = false;

    if (preloadThread.joinable())
    {
        preloadThread.join();
    }
    preloadThread = std::thread([this, filename]()
                                {
    if (stopThread){return;}

    nextLevel = std::make_shared<Level>(filename, tileSize, window); 
    nextReady = true; });
}

bool LevelManager::isNextReady() const
{
    return nextReady;
}
void LevelManager::switchToNextLevel()
{
    if (!nextReady)
    {
        return;
    }
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

        if (other.preloadThread.joinable())
        {
            other.preloadThread.join();
        }
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
