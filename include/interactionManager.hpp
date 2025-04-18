#pragma once

#include "player.hpp"
#include "gameObj.hpp"
#include <vector>

class InteractionManager
{
public:
    static void handle(Player &player, std::vector<GameObject> &objects);
};
