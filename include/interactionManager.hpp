#pragma once

#include "player.hpp"
#include "gameObj.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

class InteractionManager
{
public:
    static void handle(Player &player, std::vector<GameObject> &objects);
};
