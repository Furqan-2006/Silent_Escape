#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "mainMenu.hpp"
#include "level.hpp"
#include "player.hpp"
#include "guard.hpp"
#include "gameObj.hpp"
#include "mapLoader.hpp"
#include "interactionManager.hpp"
#include "pathfinder.hpp"
#include "levelmenu.hpp"
#include "levelmanager.hpp"
#include "textureManager.hpp"

void handleMenuInput(const sf::Event::KeyPressed &, Menu &, GameState &, sf::RenderWindow &);
void handleLevelMenuInput(const sf::Event::KeyPressed &, LevelMenu &, GameState &, sf::RenderWindow &);
void handleGameOverInput(const sf::Event::KeyPressed &);
void updateGameOverState(GameState &, sf::Clock &);
void renderMenu(sf::RenderWindow &, Menu &);
