#include "levelmenu.hpp"

LevelTile::LevelTile(float x, float y, float size)
{
    tile.setSize({size, size});
    tile.setPosition({x, y});
    tile.setFillColor(sf::Color::White);
    tile.setOutlineColor(sf::Color::Red);
    tile.setOutlineThickness(2);
}

void LevelTile::draw(sf::RenderWindow &window)
{
    window.draw(tile);
}

void LevelTile::select()
{
    isSelected = true;
    tile.setFillColor(sf::Color::Green);
}
void LevelTile::deSelect()
{
    isSelected = false;
    tile.setFillColor(sf::Color::White);
}
void LevelTile::unlock()
{
    isLocked = false;
}
bool LevelTile::getIsLocked() const
{
    return isLocked;
}

bool LevelTile::contains(sf::Vector2f &point)
{
    return tile.getGlobalBounds().contains(point);
}

// LevelMenu

LevelMenu::LevelMenu(float width, float height)
{
    selectedLevelIndex = 0;
    float tileSize = 80.f;

    if (!font.openFromFile("../assets/Fonts/CURLZ___.TTF"))
    {
        std::cerr << "Failed to load font!" << std::endl;
    }
    // std::string labels[NO_OF_LEVELS] = {"1", "2", "3"};

    for (int i = 0; i < NO_OF_LEVELS; i++)
    {
        tiles.emplace_back(100.f + i * (tileSize + 10.f), 200.f, tileSize);

        sf::Text levelLabel(font, std::to_string(i + 1), 24);
        levelLabel.setFillColor(sf::Color::Black);
        levelLabel.setPosition({100.f + i * (tileSize + 10.f) + tileSize / 4,
                                200.f + tileSize / 4});
        tileLabels.push_back(levelLabel);
    }
    tiles[0].unlock();
    tiles[1].unlock();
    tiles[2].unlock();
    tiles[0].select();
}

void LevelMenu::draw(sf::RenderWindow &window)
{
    for (auto &tile : tiles)
    {
        tile.draw(window);
    }
    for (auto &tilelabel : tileLabels)
    {
        window.draw(tilelabel);
    }
}

void LevelMenu::moveRight()
{
    if (selectedLevelIndex < NO_OF_LEVELS - 1 && !tiles[selectedLevelIndex].getIsLocked())
    {
        ++selectedLevelIndex;

        for (auto &t : tiles)
            t.deSelect();

        tiles[selectedLevelIndex].select();
    }
}

void LevelMenu::moveLeft()
{
    if (selectedLevelIndex > 0 && !tiles[selectedLevelIndex].getIsLocked())
    {
        --selectedLevelIndex;

        for (auto &t : tiles)
            t.deSelect();

        tiles[selectedLevelIndex].select();
    }
}

void LevelMenu::onClick(sf::Vector2f mousePos)
{
    for (int i = 0; i < tiles.size(); i++)
    {
        if (tiles[i].contains(mousePos))
        {
            if (!tiles[i].getIsLocked())
            {
                selectedLevelIndex = i;

                // Deselect all tiles
                for (auto &t : tiles)
                    t.deSelect();

                // Select the current tile
                tiles[selectedLevelIndex].select();
            }
        }
    }
}

int LevelMenu::getSelectedLevel() const
{
    if (!tiles[selectedLevelIndex].getIsLocked())
    {
        return selectedLevelIndex;
    }
    else
    {
        return 0;
    }
}