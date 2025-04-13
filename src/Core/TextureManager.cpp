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

    pl::VertexArray vertexArray;

    pl::Vertex vertices[4];
    
    pl::Vector2f size;
    size.x = drawData.textureRect.width * drawData.scale.x;
    size.y = drawData.textureRect.height * drawData.scale.y;

    float centreRatioX = drawData.centerRatio.x;
    float centreRatioY = drawData.centerRatio.y;

    if (drawData.useCentreAbsolute)
    {
        centreRatioX /= drawData.textureRect.width;
        centreRatioY /= drawData.textureRect.height;
    }
    
    if (drawData.rotation == 0)
    {
        // Simple case, no rotation
        // Separate from rotation calculation in order to save performance
        pl::Vector2f topLeft;
        topLeft.x = drawData.position.x - (size.x * centreRatioX);
        topLeft.y = drawData.position.y - (size.y * centreRatioY);

        vertices[0].position = topLeft;
        vertices[1].position = topLeft + pl::Vector2f(size.x, 0);
        vertices[2].position = topLeft + pl::Vector2f(size.x, size.y);
        vertices[3].position = topLeft + pl::Vector2f(0, size.y);
    }
    else
    {
        // Apply rotation
        float angleRadians = M_PI * drawData.rotation / 180.0f;

        float nX = -size.x * centreRatioX;
        float pX = (1.0f - centreRatioX) * size.x;
        float nY = -size.y * centreRatioY;
        float pY = (1.0f - centreRatioY) * size.y;

        vertices[0].position = pl::Vector2f(nX, nY).rotate(angleRadians) + drawData.position;
        vertices[1].position = pl::Vector2f(pX, nY).rotate(angleRadians) + drawData.position;
        vertices[2].position = pl::Vector2f(pX, pY).rotate(angleRadians) + drawData.position;
        vertices[3].position = pl::Vector2f(nX, pY).rotate(angleRadians) + drawData.position;
    }

    // Set UV coords
    vertices[0].textureUV = static_cast<pl::Vector2f>(drawData.textureRect.getPosition());
    vertices[1].textureUV = vertices[0].textureUV + pl::Vector2f(drawData.textureRect.width, 0);
    vertices[2].textureUV = vertices[0].textureUV + pl::Vector2f(drawData.textureRect.width, drawData.textureRect.height);
    vertices[3].textureUV = vertices[0].textureUV + pl::Vector2f(0, drawData.textureRect.height);

    for (int i = 0; i < 4; i++)
    {
        vertices[i].color = drawData.color;
    }
    
    vertexArray.addVertex(vertices[0]);
    vertexArray.addVertex(vertices[1]);
    vertexArray.addVertex(vertices[2]);
    vertexArray.addVertex(vertices[0]);
    vertexArray.addVertex(vertices[2]);
    vertexArray.addVertex(vertices[3]);

    window.draw(vertexArray, *drawData.shader, drawData.texture, pl::BlendMode::Alpha);
}