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
    for (const std::pair<TextureType, std::string>& texturePair : texturePaths)
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
        texture->setLinearFilter(false);

        // Store texture object in texture map
        textureMap[textureType] = std::move(texture);

        // Increment texture loaded count
        texturesLoaded++;
    }

    // Load bitmasks
    for (const std::pair<BitmaskType, std::string>& bitmaskPair : bitmaskPaths)
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

void TextureManager::unloadTextures()
{
    for (auto iter = textureMap.begin(); iter != textureMap.end();)
    {
        iter = textureMap.erase(iter);
    }

    for (auto iter = bitmasks.begin(); iter != bitmasks.end();)
    {
        iter = bitmasks.erase(iter);
    }
}

// Draw texture with specified data
void TextureManager::drawTexture(pl::RenderTarget& window, pl::DrawData drawData)
{
    // If not loaded textures, return by default
    if (!loadedTextures)
    {
        return;
    }

    drawData.textureRect = pl::Rect<int>(0, 0, drawData.texture->getWidth(), drawData.texture->getHeight());

    drawSubTexture(window, drawData);
}

// Draw texture using a subrectangle, useful for spritesheets and tiling textures (subrectangle bigger than texture, texture repeats)
void TextureManager::drawSubTexture(pl::RenderTarget& window, const pl::DrawData& drawData)
{
    // If not loaded textures, return by default
    if (!loadedTextures)
    {
        return;
    }

    pl::SpriteBatch spriteBatch;
    spriteBatch.draw(window, drawData);
    spriteBatch.endDrawing(window);
}