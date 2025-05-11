#include "TextureManager.hpp"

void TextureManager::load(const std::string &key, const std::string &filePath)
{
    sf::Texture tex;
    if (!tex.loadFromFile(filePath))
    {
        std::cerr << "[Error] Failed to load texture from: " << filePath << "\n";
        throw std::runtime_error("Failed to load texture from: " + filePath);
    }

    textures[key] = std::move(tex); // move to avoid copy
    std::cout << "[Debug] Loaded texture: " << filePath << " as \"" << key << "\"\n";
}

const sf::Texture &TextureManager::get(const std::string &key) const
{
    auto it = textures.find(key);
    if (it == textures.end())
    {
        throw std::runtime_error("Texture not found: " + key);
    }
    return it->second;
}

bool TextureManager::exists(const std::string &key) const
{
    return textures.find(key) != textures.end();
}
