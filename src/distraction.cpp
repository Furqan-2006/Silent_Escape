#include "distraction.hpp"

// Constructor for the Distraction class, initializing the stone object with position, radius, and duration.
Distraction::Distraction(const sf::Vector2f &pos, float radius, float dur) : duration(dur)
{
    // Set the radius of the stone (a circle shape).
    stone.setRadius(radius);

    // Set the fill color of the stone with a semi-transparent red color.
    stone.setFillColor(sf::Color(255, 0, 0, 80)); // Red with 80 alpha (transparency)

    // Set the position of the stone.
    stone.setPosition(pos);

    // Set the origin of the stone to its center (for correct rotation and scaling).
    stone.setOrigin({radius, radius});
}

// This function checks if the distraction has expired (i.e., if the duration has passed).
bool Distraction::isExpired() const
{
    // Returns true if the elapsed time exceeds the duration, meaning the distraction is expired.
    return clock.getElapsedTime().asSeconds() > duration;
}

// This function draws the stone (distraction) on the given window.
void Distraction::draw(sf::RenderWindow &window)
{
    // Draw the stone object onto the window.
    window.draw(stone);
}

// Getter function to return the position of the stone.
sf::Vector2f Distraction::getPos() const
{
    // Return the current position of the stone.
    return stone.getPosition();
}

// Getter function to return the radius of the stone.
float Distraction::getRadius() const
{
    // Return the radius of the stone.
    return stone.getRadius();
}
