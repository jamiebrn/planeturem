#include "Core/TextureManager.hpp"

// Initialise member variables, as is static class
bool TextureManager::loadedTextures = false;

// Stores loaded textures
std::unordered_map<TextureType, std::unique_ptr<pl::Texture>> TextureManager::textureMap;

// Stores sprites
// std::unordered_map<TextureType, pl::Sprite> TextureManager::spriteMap;

std::unordered_map<BitmaskType, std::unique_ptr<pl::Image>> TextureManager::bitmasks;

// All file paths for textures are listed here
const std::unordered_map<TextureType, std::string> TextureManager::texturePaths = {
    {TextureType::Player, "Data/Textures/monkeyplayer.png"},
    {TextureType::SelectTile, "Data/Textures/select_tile_animated.png"},
    {TextureType::GroundTiles, "Data/Textures/tiles.png"},
    {TextureType::Water, "Data/Textures/water.png"},
    {TextureType::Items, "Data/Textures/items.png"},
    {TextureType::Objects, "Data/Textures/objects.png"},
    {TextureType::Entities, "Data/Textures/entities.png"},
    {TextureType::Tools, "Data/Textures/tools.png"},
    {TextureType::Shadow, "Data/Textures/shadow.png"},
    // {TextureType::LightMask, "Data/Textures/light_mask.png"},
    {TextureType::UI, "Data/Textures/UI.png"},
    {TextureType::Rooms, "Data/Textures/rooms.png"},
    {TextureType::Portraits, "Data/Textures/portraits.png"}
};

const std::unordered_map<BitmaskType, std::string> TextureManager::bitmaskPaths = {
    {BitmaskType::Structures, "Data/Textures/collision_bitmasks.png"}
};

// Loads all textures from paths specified into texture map
bool TextureManager::loadTextures()
{
    // If textures already loaded, return true by default
    if (loadedTextures)
        return true;
    
    // Set loaded textures to true by default
    loadedTextures = true;

    // Count of how many textures have been loaded
    float texturesLoaded = 0;

    // Iterate over each texture file path
    for (std::pair<TextureType, std::string> texturePair : texturePaths)
    {
        // Get texture type and file path from map
        TextureType textureType = texturePair.first;
        std::string texturePath = texturePair.second;

        // Create texture object
        std::unique_ptr<pl::Texture> texture = std::make_unique<pl::Texture>();

        // Attempt to load stream into texture object
        if (!texture->loadTexture(texturePath))
        {
            // If failed, set loaded textures to false
            loadedTextures = false;
            // Stop loading textures
            break;
        }

        // Set texture repeating (tiling) to true by default
        texture->setTextureRepeat(true);

        // Store texture object in texture map
        textureMap[textureType] = std::move(texture);

        // Create sprite object to interface with the texture
        // sf::Sprite sprite;

        // Set sprite texture from texture map
        // sprite.setTexture(textureMap[textureType]);

        // Store sprite in sprite map
        // spriteMap[textureType] = sprite;

        // Increment texture loaded count
        texturesLoaded++;
    }

    // Load bitmasks
    for (std::pair<BitmaskType, std::string> bitmaskPair : bitmaskPaths)
    {
        std::unique_ptr<pl::Image> bitmaskImage = std::make_unique<pl::Image>();

        if (!bitmaskImage->loadFromFile(bitmaskPair.second))
        {
            // If failed, set loaded textures to false
            loadedTextures = false;
            // Stop loading textures
            break;
        }

        bitmasks[bitmaskPair.first] = std::move(bitmaskImage);

        // Increment texture loaded count
        texturesLoaded++;
    }

    // If textures not loaded successfully, return false
    if (!loadedTextures)
        return false;
    
    // Return true by default
    return true;
}

// Draw texture with specified data
void TextureManager::drawTexture(pl::RenderTarget& window, const TextureDrawData& drawData, const pl::Shader& shader)
{
    // If not loaded textures, return by default
    if (!loadedTextures)
    {
        return;
    }

    const pl::Texture& texture = *textureMap[drawData.type].get();

    drawSubTexture(window, drawData, pl::Rect<float>(0, 0, texture.getWidth(), texture.getHeight()), shader);
    
    // Get sprite from sprite map
    // sf::Sprite& sprite = spriteMap.at(drawData.type);

    // Apply draw data to texture
    // applyTextureData(drawData);

    // window.draw(sprite, renderState);
}

// Draw texture using a subrectangle, useful for spritesheets and tiling textures (subrectangle bigger than texture, texture repeats)
void TextureManager::drawSubTexture(pl::RenderTarget& window, const TextureDrawData& drawData, pl::Rect<float> boundRect, const pl::Shader& shader)
{
    // If not loaded textures, return by default
    if (!loadedTextures)
    {
        return;
    }

    // Get sprite from sprite map
    // sf::Sprite& sprite = spriteMap.at(drawData.type);

    // Apply subrectangle to sprite
    // sprite.setTextureRect(boundRect);

    // Apply draw data to texture
    // applyTextureData(drawData);

    pl::Rect<float> quad;
    if (drawData.useCentreAbsolute)
    {
        quad.x = drawData.position.x - drawData.centerRatio.x;
        quad.y = drawData.position.y - drawData.centerRatio.y;
    }
    else
    {
        quad.x = drawData.position.x - boundRect.width * drawData.centerRatio.x;
        quad.y = drawData.position.y - boundRect.height * drawData.centerRatio.y;
    }

    quad.width = boundRect.width;
    quad.height = boundRect.height;

    const pl::Texture& texture = *textureMap[drawData.type].get();

    boundRect.x /= texture.getWidth();
    boundRect.y /= texture.getHeight();
    boundRect.width /= texture.getWidth();
    boundRect.height /= texture.getHeight();

    pl::VertexArray vertexArray;
    vertexArray.addQuad(quad, drawData.colour, boundRect);

    window.draw(vertexArray, shader, texture, pl::BlendMode::Alpha);
    // Draw with shader if required
    // if (shader)
    // {
    //     return;
    // }

    // // Draw sprite
    // window.draw(sprite);

}

// Apply draw data before drawing a texture
// void TextureManager::applyTextureData(TextureDrawData drawData)
// {
//     // Get sprite from sprite map
//     sf::Sprite& sprite = spriteMap.at(drawData.type);

//     // Set scale of sprite from draw data
//     sprite.setScale(drawData.scale);

//     // Get size of sprite
//     sf::FloatRect sizeRect = sprite.getLocalBounds();
//     // Calculate middle point of sprite
//     sf::Vector2f origin = drawData.useCentreAbsolute ? drawData.centerRatio : sf::Vector2f(sizeRect.width * drawData.centerRatio.x, sizeRect.height * drawData.centerRatio.y);
//     // Set origin of sprite to middle
//     sprite.setOrigin(origin);

//     // Set position and rotation from draw data
//     sprite.setPosition(drawData.position);
//     sprite.setRotation(drawData.rotation);

//     // Set colour from draw data
//     sprite.setColor(drawData.colour);
// }