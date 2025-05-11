#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <iostream>

class TextureManager
{
public:
    // Loads a texture from file and stores it with a key
    void load(const std::string &key, const std::string &filePath);

    // Returns a reference to the texture associated with the key
    const sf::Texture &get(const std::string &key) const;

    // Checks if texture exists
    bool exists(const std::string &key) const;

private:
    std::unordered_map<std::string, sf::Texture> textures;
};