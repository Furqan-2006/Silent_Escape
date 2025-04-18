#include "interactionManager.hpp"
#include <iostream>

void InteractionManager::handle(Player &player, std::vector<GameObject> &objects)
{
    for (auto &obj : objects)
    {
        float dx = player.getPosition().x - obj.getPosition().x;
        float dy = player.getPosition().y - obj.getPosition().y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (player.getBounds().findIntersection(obj.getBounds()).has_value() || dist < 30.f)
        {
            switch (obj.getType())
            {
            case ObjectType::Door:
                player.hack();
                obj.setFillColor(sf::Color::Red);
                std::cout << "[LOG] Player is interacting with a door\n";
                return;
            case ObjectType::Box:
                player.hide();
                std::cout << "[LOG] Player is interacting with a Box\n";
                return;
            case ObjectType::Disguise:
                player.disguise();
                std::cout << "[LOG] Player is disguising \n";
                return;
            case ObjectType::Terminal:
                player.hack();
                std::cout << "[LOG] Player is interacting with a terminal\n";
                return;
            }
            return;
        }
    }
}
