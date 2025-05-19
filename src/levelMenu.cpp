#include "levelmenu.hpp"

// LevelTile Class Definition

// Constructor to initialize a level tile at a specified position and size
LevelTile::LevelTile(float x, float y, float size)
{
    tile.setSize({size, size});  // Set the tile size
    tile.setPosition({x, y});  // Set the position of the tile
    tile.setFillColor(sf::Color::White);  // Default fill color (White)
    tile.setOutlineColor(sf::Color::Red);  // Set the outline color (Red)
    tile.setOutlineThickness(2);  // Set the outline thickness
}

// Draw the tile to the window
void LevelTile::draw(sf::RenderWindow &window)
{
    window.draw(tile);  // Draw the tile shape
}

// Select the current tile, changing its color to green to indicate selection
void LevelTile::select()
{
    isSelected = true;  // Set the tile as selected
    tile.setFillColor(sf::Color::Green);  // Change the tile's fill color to Green
}

// Deselect the current tile, reverting its color to white
void LevelTile::deSelect()
{
    isSelected = false;  // Set the tile as deselected
    tile.setFillColor(sf::Color::White);  // Revert the tile's fill color to White
}

// Unlock the tile, allowing interaction with it
void LevelTile::unlock()
{
    isLocked = false;  // Set the tile as unlocked
}

// Return whether the tile is locked or not
bool LevelTile::getIsLocked() const
{
    return isLocked;  // Return the status of the tile (locked or unlocked)
}

// Check if the tile contains a given point (usually the mouse position)
bool LevelTile::contains(sf::Vector2f &point)
{
    return tile.getGlobalBounds().contains(point);  // Check if the point is inside the tile's bounds
}

// LevelMenu Class Definition

// Constructor to initialize the level menu
LevelMenu::LevelMenu(float width, float height)
{
    selectedLevelIndex = 0;  // Start with the first level selected
    float tileSize = 80.f;  // Size of each tile

    // Try to load the font for the level labels
    if (!font.openFromFile("../assets/Fonts/CURLZ___.TTF"))
    {
        std::cerr << "Failed to load font!" << std::endl;  // Error if font cannot be loaded
    }

    // Create tiles for the levels and their labels
    for (int i = 0; i < NO_OF_LEVELS; i++)
    {
        // Create a tile for each level
        tiles.emplace_back(100.f + i * (tileSize + 10.f), 200.f, tileSize);

        // Create a label for each tile (level number)
        sf::Text levelLabel(font, std::to_string(i + 1), 24);  // Set label text (level number)
        levelLabel.setFillColor(sf::Color::Black);  // Set label text color (Black)
        levelLabel.setPosition({100.f + i * (tileSize + 10.f) + tileSize / 4,  // Position the label
                                200.f + tileSize / 4});
        tileLabels.push_back(levelLabel);  // Add label to the list of labels
    }

    // Unlock the first five levels (allow interaction with them)
    tiles[0].unlock();
    tiles[1].unlock();
    tiles[2].unlock();
    tiles[3].unlock();
    tiles[4].unlock();
    tiles[0].select();  // Select the first level by default
}

// Function to draw the level menu (tiles and labels) to the window
void LevelMenu::draw(sf::RenderWindow &window)
{
    // Draw all the tiles
    for (auto &tile : tiles)
    {
        tile.draw(window);
    }

    // Draw all the level labels
    for (auto &tilelabel : tileLabels)
    {
        window.draw(tilelabel);
    }
}

// Function to move the selection to the right (next level) if possible
void LevelMenu::moveRight()
{
    if (selectedLevelIndex < NO_OF_LEVELS - 1 && !tiles[selectedLevelIndex].getIsLocked())  // Check if next level is available and unlocked
    {
        ++selectedLevelIndex;  // Move the selection to the right

        // Deselect all tiles
        for (auto &t : tiles)
            t.deSelect();

        tiles[selectedLevelIndex].select();  // Select the new tile
    }
}

// Function to move the selection to the left (previous level) if possible
void LevelMenu::moveLeft()
{
    if (selectedLevelIndex > 0 && !tiles[selectedLevelIndex].getIsLocked())  // Check if previous level is available and unlocked
    {
        --selectedLevelIndex;  // Move the selection to the left

        // Deselect all tiles
        for (auto &t : tiles)
            t.deSelect();

        tiles[selectedLevelIndex].select();  // Select the new tile
    }
}

// Function to handle mouse click event and select a tile if clicked
void LevelMenu::onClick(sf::Vector2f mousePos)
{
    for (int i = 0; i < tiles.size(); i++)
    {
        if (tiles[i].contains(mousePos))  // If the mouse position is inside the tile's bounds
        {
            if (!tiles[i].getIsLocked())  // If the tile is not locked
            {
                selectedLevelIndex = i;  // Set the selected level index to the clicked tile

                // Deselect all tiles
                for (auto &t : tiles)
                    t.deSelect();

                // Select the clicked tile
                tiles[selectedLevelIndex].select();
            }
        }
    }
}

// Function to get the index of the selected level
int LevelMenu::getSelectedLevel() const
{
    if (!tiles[selectedLevelIndex].getIsLocked())  // If the selected level is not locked
    {
        return selectedLevelIndex;  // Return the selected level index
    }
    else
    {
        return 0;  // If the selected level is locked, return 0 (indicating no selection)
    }
}
