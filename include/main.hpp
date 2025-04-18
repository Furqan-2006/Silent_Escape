#pragma once

#include <SFML/Graphics.hpp>
#include "mainMenu.hpp"
#include "player.hpp"
#include "guard.hpp"
#include "gameObj.hpp"
#include "mapLoader.hpp"
#include "interactionManager.hpp"

enum class GameState
{
    MENU,
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    GAME_OVER,
    EXIT
};

void handleMenuInput(const sf::Event::KeyPressed &, Menu &, GameState &, sf::RenderWindow &);
void handleLevel1Input(const sf::Event::KeyPressed &, Player &, std::vector<GameObject> &);
void updateGameOverState(GameState &, sf::Clock &);
void renderMenu(sf::RenderWindow &, Menu &);
void renderLevel1(sf::RenderWindow &, Player &, Guard &, std::vector<GameObject> &, GameState &);
