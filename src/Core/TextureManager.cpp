#include "Core/TextureManager.hpp"

// Initialise member variables, as is static class
bool TextureManager::loadedTextures = false;

// Stores loaded textures
std::unordered_map<TextureType, sf::Texture> TextureManager::textureMap;

// Stores sprites
std::unordered_map<TextureType, sf::Sprite> TextureManager::spriteMap;

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
    {TextureType::LightMask, "Data/Textures/light_mask.png"},
    {TextureType::UI, "Data/Textures/UI.png"},
    {TextureType::CollisionBitmask, "Data/Textures/collision_bitmasks.png"}
};

// Loads all textures from paths specified into texture map
bool TextureManager::loadTextures(sf::RenderWindow& window)
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
        sf::Texture texture;

        // Attempt to load stream into texture object
        if (!texture.loadFromFile(texturePath))
        {
            // If failed, set loaded textures to false
            loadedTextures = false;
            // Stop loading textures
            break;
        }

        // Set texture repeating (tiling) to true by default
        texture.setRepeated(true);

        // Store texture object in texture map
        textureMap[textureType] = texture;

        // Create sprite object to interface with the texture
        sf::Sprite sprite;

        // Set sprite texture from texture map
        sprite.setTexture(textureMap[textureType]);

        // Store sprite in sprite map
        spriteMap[textureType] = sprite;

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
void TextureManager::drawTexture(sf::RenderTarget& window, TextureDrawData drawData, const sf::RenderStates& renderState)
{
    // If not loaded textures, return by default
    if (!loadedTextures)
        return;
    
    // Get sprite from sprite map
    sf::Sprite& sprite = spriteMap.at(drawData.type);

    // Apply draw data to texture
    applyTextureData(drawData);

    window.draw(sprite, renderState);

    // Draw with shader if required
    // if (shader)
    // {
    //     window.draw(sprite, shader);
    //     return;
    // }

    // // Draw sprite
    // window.draw(sprite);
}

// Draw texture using a subrectangle, useful for spritesheets and tiling textures (subrectangle bigger than texture, texture repeats)
void TextureManager::drawSubTexture(sf::RenderTarget& window, TextureDrawData drawData, sf::IntRect boundRect, const sf::RenderStates& renderState)
{
    // If not loaded textures, return by default
    if (!loadedTextures)
        return;

    // Get sprite from sprite map
    sf::Sprite& sprite = spriteMap.at(drawData.type);

    // Apply subrectangle to sprite
    sprite.setTextureRect(boundRect);

    // Apply draw data to texture
    applyTextureData(drawData);
    
    window.draw(sprite, renderState);
    // Draw with shader if required
    // if (shader)
    // {
    //     return;
    // }

    // // Draw sprite
    // window.draw(sprite);

}

// Apply draw data before drawing a texture
void TextureManager::applyTextureData(TextureDrawData drawData)
{
    // Get sprite from sprite map
    sf::Sprite& sprite = spriteMap.at(drawData.type);

    // Set scale of sprite from draw data
    sprite.setScale(drawData.scale);

    // Get size of sprite
    sf::FloatRect sizeRect = sprite.getLocalBounds();
    // Calculate middle point of sprite
    sf::Vector2f origin = sf::Vector2f(sizeRect.width * drawData.centerRatio.x, sizeRect.height * drawData.centerRatio.y);
    // Set origin of sprite to middle
    sprite.setOrigin(origin);

    // Set position and rotation from draw data
    sprite.setPosition(drawData.position);
    sprite.setRotation(drawData.rotation);

    // Set colour from draw data
    sprite.setColor(drawData.colour);

}