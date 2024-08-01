#pragma once

// Include libraries
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

#include "Types/TextureType.hpp"

// Struct containing data required to draw texture
struct TextureDrawData
{
    // Type of texture
    TextureType type;
    // Draw position on screen
    sf::Vector2f position;
    // Rotation
    float rotation;
    // Scale
    sf::Vector2f scale;
    // Whether texture should be drawn centred about its position
    sf::Vector2f centerRatio = sf::Vector2f(0, 0);
    // The base colour the texture should be drawn in (white in most cases)
    sf::Color colour = sf::Color(255, 255, 255);
};

// Declaration of TextureManager class
class TextureManager
{

// Private TextureManager constructor, so class cannot be instantiated
// Creates static class behaviour
private:
    TextureManager() = delete;

// Public functions
public:
    // Load all textures into memory
    static bool loadTextures(sf::RenderWindow& window);

    // Draw texture using draw data
    static void drawTexture(sf::RenderTarget& window, TextureDrawData drawData, sf::Shader* shader = nullptr);

    // Draw a section of a texture using draw data
    static void drawSubTexture(sf::RenderTarget& window, TextureDrawData drawData, sf::IntRect boundRect, sf::Shader* shader = nullptr);

    // Get the size of a specific texture (width x height)
    inline static sf::Vector2u getTextureSize(TextureType type) {return textureMap[type].getSize();}

    inline static sf::Texture* getTexture(TextureType type) {return &textureMap[type];}

// Private functions
private:
    // Apply draw data before drawing a texture
    static void applyTextureData(TextureDrawData drawData);

// Private member variables
private:
    // Stores whether textures have been loaded
    static bool loadedTextures;

    // Stores loaded textures
    static std::unordered_map<TextureType, sf::Texture> textureMap;

    // Stores sprites, which provide an interface over the textures
    static std::unordered_map<TextureType, sf::Sprite> spriteMap;

    // Stores file path to each texture, so each texture can be loaded
    static const std::unordered_map<TextureType, std::string> texturePaths;

};
