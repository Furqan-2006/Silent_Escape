#include "distraction.hpp"

Distraction::Distraction(const sf::Vector2f &pos, float radius, float dur) : duration(dur)
{
    stone.setRadius(radius);
    stone.setFillColor(sf::Color(255, 0, 0, 80));
    // sf::Vector2f isoPos =
    stone.setPosition(pos);
    stone.setOrigin({radius, radius});
}

bool Distraction::isExpired() const
{
    return clock.getElapsedTime().asSeconds() > duration;
}
void Distraction::draw(sf::RenderWindow &window)
{
    window.draw(stone);
}
sf::Vector2f Distraction::getPos() const
{
    return stone.getPosition();
}
float Distraction::getRadius() const
{
    return stone.getRadius();
}