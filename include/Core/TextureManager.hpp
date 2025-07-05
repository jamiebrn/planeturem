#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <Graphics/RenderTarget.hpp>
#include <Graphics/Image.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/VertexArray.hpp>
#include <Graphics/SpriteBatch.hpp>
#include <Graphics/DrawData.hpp>
#include <Rect.hpp>
#include <Vector.hpp>

#include <unordered_map>
#include <string>
#include <memory>

#include "Types/TextureType.hpp"

class TextureManager
{
private:
    TextureManager() = delete;

public:
    static bool loadTextures();

    static void unloadTextures();

    // Draw texture using draw data
    static void drawTexture(pl::RenderTarget& window, pl::DrawData drawData);

    // Draw a section of a texture using draw data
    static void drawSubTexture(pl::RenderTarget& window, const pl::DrawData& drawData);

    // Get the size of a specific texture (width x height)
    inline static pl::Vector2<int> getTextureSize(TextureType type) {return pl::Vector2<int>(textureMap[type]->getWidth(), textureMap[type]->getHeight());}

    inline static pl::Texture* getTexture(TextureType type) {return textureMap[type].get();}

    inline static const pl::Image& getBitmask(BitmaskType type) {return *bitmasks[type].get();}
    
    static inline const std::string& getTextureHash() {return textureHash;}

private:
    static bool loadedTextures;

    static std::unordered_map<TextureType, std::unique_ptr<pl::Texture>> textureMap;

    static const std::unordered_map<TextureType, std::string> texturePaths;

    static std::unordered_map<BitmaskType, std::unique_ptr<pl::Image>> bitmasks;

    static const std::unordered_map<BitmaskType, std::string> bitmaskPaths;

    static std::string textureHash;

};