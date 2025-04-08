#pragma once

// Include libraries
// #include <SFML/Graphics.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Image.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/VertexArray.hpp>
#include <Graphics/DrawData.hpp>
#include <Rect.hpp>
#include <Vector.hpp>

#include <unordered_map>
#include <string>
#include <memory>

#include "Types/TextureType.hpp"

// Struct containing data required to draw texture
// struct TextureDrawData
// {
//     // Type of texture
//     TextureType type;
//     // Draw position on screen
//     pl::Vector2f position;
//     // Rotation
//     float rotation = 0.0f;
//     // Scale
//     pl::Vector2f scale;
//     // Whether texture should be drawn centred about its position
//     pl::Vector2f centerRatio = pl::Vector2f(0, 0);
//     // The base colour the texture should be drawn in (white in most cases)
//     pl::Color colour = pl::Color(255, 255, 255);
    
//     bool useCentreAbsolute = false;
// };

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
    static bool loadTextures();

    // Draw texture using draw data
    static void drawTexture(pl::RenderTarget& window, pl::DrawData drawData);

    // Draw a section of a texture using draw data
    static void drawSubTexture(pl::RenderTarget& window, const pl::DrawData& drawData);

    // Get the size of a specific texture (width x height)
    inline static pl::Vector2<int> getTextureSize(TextureType type) {return pl::Vector2<int>(textureMap[type]->getWidth(), textureMap[type]->getHeight());}

    inline static pl::Texture* getTexture(TextureType type) {return textureMap[type].get();}

    inline static const pl::Image& getBitmask(BitmaskType type) {return *bitmasks[type].get();}

// Private functions
private:
    // Apply draw data before drawing a texture
    // static void applyTextureData(TextureDrawData drawData);

// Private member variables
private:
    // Stores whether textures have been loaded
    static bool loadedTextures;

    // Stores loaded textures
    static std::unordered_map<TextureType, std::unique_ptr<pl::Texture>> textureMap;

    // Stores sprites, which provide an interface over the textures
    // static std::unordered_map<TextureType, sf::Sprite> spriteMap;

    // Stores file path to each texture, so each texture can be loaded
    static const std::unordered_map<TextureType, std::string> texturePaths;

    static std::unordered_map<BitmaskType, std::unique_ptr<pl::Image>> bitmasks;

    static const std::unordered_map<BitmaskType, std::string> bitmaskPaths;

};