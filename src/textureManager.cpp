#include "TextureManager.hpp"

// Load a texture from a file and store it in the textures map using the specified key
void TextureManager::load(const std::string &key, const std::string &filePath)
{
    sf::Texture tex; // Create a texture object

    // Try to load the texture from the specified file path
    if (!tex.loadFromFile(filePath))
    {
        std::cerr << "[Error] Failed to load texture from: " << filePath << "\n"; // Print error message if loading fails
        throw std::runtime_error("Failed to load texture from: " + filePath);     // Throw an exception if loading fails
    }

    // Store the loaded texture in the textures map using the given key
    textures[key] = std::move(tex);                                                   // Use move semantics to avoid unnecessary copy
    std::cout << "[Debug] Loaded texture: " << filePath << " as \"" << key << "\"\n"; // Debug log to confirm texture load
}

// Retrieve a texture by its key from the textures map
const sf::Texture &TextureManager::get(const std::string &key) const
{
    // Find the texture in the map by the key
    auto it = textures.find(key);
    if (it == textures.end()) // If the texture is not found
    {
        throw std::runtime_error("Texture not found: " + key); // Throw an exception indicating texture not found
    }
    return it->second; // Return the found texture (by reference)
}

// Check if a texture with the given key exists in the textures map
bool TextureManager::exists(const std::string &key) const
{
    return textures.find(key) != textures.end(); // Return true if the texture is found, false otherwise
}
