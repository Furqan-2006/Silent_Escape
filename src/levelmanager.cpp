#include "levelmanager.hpp"

// Constructor that initializes the LevelManager with a reference to the window and the tile size
LevelManager::LevelManager(sf::RenderWindow &window, float tileSize)
    : nextReady(false), stopThread(false), window(window), tileSize(tileSize) {}

// Destructor that ensures the thread is stopped before the object is destroyed
LevelManager::~LevelManager()
{
    stopThread = true;            // Set the flag to stop the thread
    if (preloadThread.joinable()) // If the preload thread is still running, join it
    {
        preloadThread.join(); // Wait for the thread to finish
    }
}

// Function to load the initial level based on the provided filename and texture manager
void LevelManager::loadInitialLevel(const std::string &filename, TextureManager &textureManager)
{
    currentLevel = std::make_shared<Level>(filename, textureManager, tileSize, window); // Load and set the current level
}

// Function to preload the next level in a separate thread
void LevelManager::preloadNextLevel(const std::string &filename, TextureManager &textureManager)
{
    nextReady = false; // Reset nextReady flag before preloading

    // If there was a previous preload thread running, wait for it to finish
    if (preloadThread.joinable())
    {
        preloadThread.join(); // Join the previous thread to avoid race conditions
    }

    // Create a new thread to load the next level
    preloadThread = std::thread([this, filename, textureManagerPtr = &textureManager]()
                                {
    if (stopThread.load()) return;  // If stopThread flag is set, return immediately

    auto loadedLevel = std::make_shared<Level>(filename, *textureManagerPtr, tileSize, window);  // Load the next level
    if (!stopThread.load())  // If stopThread flag is not set
    {
        nextLevel = loadedLevel;  // Set the next level
        nextReady = true;  // Indicate that the next level is ready
    } });
}

// Function to check if the next level has been preloaded and is ready to be switched to
bool LevelManager::isNextReady() const
{
    return nextReady.load(); // Return the status of the nextReady flag
}

// Function to switch to the next level if it's ready
void LevelManager::switchToNextLevel()
{
    if (!nextReady.load()) // If the next level is not ready, do nothing
        return;

    currentLevel = nextLevel; // Switch to the next level
    nextLevel.reset();        // Reset the next level pointer
    nextReady = false;        // Reset the nextReady flag
}

// Function to get the current level
std::shared_ptr<Level> LevelManager::getCurrentLevel()
{
    return currentLevel; // Return the current level
}

// Move constructor that transfers ownership of resources from another LevelManager object
LevelManager::LevelManager(LevelManager &&other) noexcept
    : currentLevel(std::move(other.currentLevel)),
      nextLevel(std::move(other.nextLevel)),
      nextReady(other.nextReady.load()),
      stopThread(false),
      window(other.window),
      tileSize(other.tileSize)
{
    // If the other object had a running preload thread, wait for it to finish
    if (other.preloadThread.joinable())
    {
        other.preloadThread.join();
    }
    preloadThread = std::move(other.preloadThread); // Move the preload thread
}

// Move assignment operator that transfers ownership of resources from another LevelManager object
LevelManager &LevelManager::operator=(LevelManager &&other) noexcept
{
    if (this != &other) // Ensure not assigning to itself
    {
        stopThread = true; // Stop the thread to safely transfer resources
        if (preloadThread.joinable())
        {
            preloadThread.join(); // Wait for the current preload thread to finish
        }

        // Transfer ownership of the resources from 'other' to 'this'
        currentLevel = std::move(other.currentLevel);
        nextLevel = std::move(other.nextLevel);
        nextReady = other.nextReady.load();
        stopThread = false;

        preloadThread = std::move(other.preloadThread); // Move the preload thread
    }
    return *this; // Return the current object
}

// Function to reset the LevelManager, stopping any ongoing preload thread and clearing the current levels
void LevelManager::reset()
{
    stopThread = true; // Set the flag to stop the thread
    if (preloadThread.joinable())
    {
        preloadThread.join(); // Wait for the preload thread to finish
    }
    stopThread = false;   // Reset the stopThread flag
    currentLevel.reset(); // Reset the current level
    nextLevel.reset();    // Reset the next level
    nextReady = false;    // Reset the nextReady flag
}
