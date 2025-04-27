#pragma once

#include <SFML/Graphics.hpp>
#include "mainMenu.hpp"
#include "level.hpp"
#include "player.hpp"
#include "guard.hpp"
#include "gameObj.hpp"
#include "mapLoader.hpp"
#include "interactionManager.hpp"
#include "pathfinder.hpp"
#include "levelmenu.hpp"

void handleMenuInput(const sf::Event::KeyPressed &, Menu &, GameState &, sf::RenderWindow &);
void handleLevelMenuInput(const sf::Event::KeyPressed &, LevelMenu &, GameState &, sf::RenderWindow &);
void handleLevel1Input(const sf::Event::KeyPressed &, Player &, std::vector<GameObject> &);
void updateGameOverState(GameState &, sf::Clock &);
void renderMenu(sf::RenderWindow &, Menu &);
void renderLevel1(sf::RenderWindow &, Player &, Guard &, std::vector<GameObject> &, GameState &, PathFinder &);
