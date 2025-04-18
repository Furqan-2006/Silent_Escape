#include "interactionManager.hpp"
#include <iostream>

void InteractionManager::handle(Player &player, std::vector<GameObject> &objects)
{
    for (auto &obj : objects)
    {
        if (player.getBounds().findIntersection(obj.getBounds()).has_value())
        {
            switch (obj.getType())
            {
            case ObjectType::Door:
                std::cout << "[LOG] Player is interacting with a door\n";
                // Open or unlock the door
                break;
            case ObjectType::Box:
                std::cout << "[LOG] Player is interacting with a Box\n";
                player.hide();
                break;
            case ObjectType::Disguise:
                std::cout << "[LOG] Player is disguising \n";
                player.disguise();
                break;
            case ObjectType::Terminal:
                std::cout << "[LOG] Player is interacting with a terminal\n";
                // player.hack();
                break;
            }
            return;
        }
    }
}
