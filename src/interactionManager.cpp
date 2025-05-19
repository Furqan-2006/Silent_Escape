#include "interactionManager.hpp"
#include <iostream>

// Function to handle interactions between the player and game objects
void InteractionManager::handle(Player &player, std::vector<GameObject> &objects)
{
    // Check if the "E" key is pressed for interaction
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
        return; // Exit the function if the "E" key is not pressed

    // Iterate through all game objects to check for interaction
    for (auto &obj : objects)
    {
        // Calculate the distance between the player and the object
        float dx = player.getPos().x - obj.getPos().x;
        float dy = player.getPos().y - obj.getPos().y;
        float dist = std::sqrt(dx * dx + dy * dy); // Calculate Euclidean distance

        // Check if the player is within collision range of the object or within 30 units
        if ((player.getBounds().findIntersection(obj.getBounds()).has_value() || dist < 30.f))
        {
            // Depending on the type of object, trigger a specific player action
            switch (obj.getType())
            {
            case ObjectType::Door:
                player.hack(); // Player hacks the door
                std::cout << "[LOG] Player is interacting with a door\n";
                return; // Exit the function after the interaction
            case ObjectType::Box:
                player.hide(); // Player hides inside the box
                std::cout << "[LOG] Player is interacting with a Box\n";
                return; // Exit the function after the interaction
            case ObjectType::Disguise:
                player.disguise(); // Player uses disguise
                std::cout << "[LOG] Player is disguising\n";
                return; // Exit the function after the interaction
            case ObjectType::Terminal:
                player.hack(); // Player hacks the terminal
                std::cout << "[LOG] Player is interacting with a terminal\n";
                return; // Exit the function after the interaction
            }
            return; // Exit the function if no match is found
        }
    }
}
