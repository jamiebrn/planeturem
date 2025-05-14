#include "Object/StructureObject.hpp"

StructureObject::StructureObject(pl::Vector2f position, StructureType structureType)
    : WorldObject(position)
{
    this->structureType = structureType;
}

void StructureObject::createWarpRect(pl::Vector2f rectPosition)
{
    if (warpEntranceRect.has_value())
        return;
    
    warpEntranceRect = CollisionRect();
    warpEntranceRect->x = rectPosition.x;
    warpEntranceRect->y = rectPosition.y;
    warpEntranceRect->width = TILE_SIZE_PIXELS_UNSCALED;
    warpEntranceRect->height = TILE_SIZE_PIXELS_UNSCALED;
}

bool StructureObject::isPlayerInEntrance(pl::Vector2f playerPos)
{
    if (!warpEntranceRect.has_value())
        return false;

    return warpEntranceRect->isPointInRect(playerPos.x, playerPos.y);
}

pl::Vector2f StructureObject::getEntrancePosition()
{
    return pl::Vector2f(warpEntranceRect->x, warpEntranceRect->y);
}

void StructureObject::setWorldPosition(pl::Vector2f newPosition)
{
    pl::Vector2f entranceRelativePos = pl::Vector2f(warpEntranceRect->x, warpEntranceRect->y) - position;

    warpEntranceRect->x = newPosition.x + entranceRelativePos.x;
    warpEntranceRect->y = newPosition.y + entranceRelativePos.y;

    position = newPosition;
}

void StructureObject::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const pl::Color& color) const
{
    const StructureData& structureData = StructureDataLoader::getStructureData(structureType);

    pl::DrawData drawData;
    drawData.texture = TextureManager::getTexture(TextureType::Objects);
    drawData.shader = Shaders::getShader(ShaderType::Default);
    drawData.position = camera.worldToScreenTransform(position, worldSize);
    drawData.scale = pl::Vector2f(ResolutionHandler::getScale(), ResolutionHandler::getScale());
    drawData.centerRatio = structureData.textureOrigin;
    drawData.textureRect = structureData.textureRect;
    
    spriteBatch.draw(window, drawData);
}

void StructureObject::createLightSource(LightingEngine& lightingEngine, pl::Vector2f topLeftChunkPos) const
{
    const pl::Image& bitmaskImage = TextureManager::getBitmask(BitmaskType::Structures);

    const StructureData& structureData = StructureDataLoader::getStructureData(structureType);
    
    pl::Vector2f relativePos = position - topLeftChunkPos;

    // Lighting tile is top left of structure
    int lightingTileX = std::floor(relativePos.x / TILE_SIZE_PIXELS_UNSCALED) * TILE_LIGHTING_RESOLUTION;
    int lightingTileY = std::floor(relativePos.y / TILE_SIZE_PIXELS_UNSCALED) * TILE_LIGHTING_RESOLUTION - (structureData.size.y - 1) * TILE_LIGHTING_RESOLUTION;

    // Iterate over lighting bitmask and add light emitter / absorber based on sample
    for (int x = 0; x < structureData.size.x; x++)
    {
        for (int y = 0; y < structureData.size.y; y++)
        {
            pl::Color bitmaskColor = bitmaskImage.getPixel(structureData.lightBitmaskOffset.x + x, structureData.lightBitmaskOffset.y + y);

            void (LightingEngine::*lightingFunction)(int, int, float) = nullptr;
            float lightingValue = 0.0f;

            if (bitmaskColor.b > 0)
            {
                lightingFunction = &LightingEngine::addLightSource;
                lightingValue = bitmaskColor.b / 255.0f;
            }
            else if (bitmaskColor.r > 0)
            {
                lightingFunction = &LightingEngine::addObstacle;
                lightingValue = bitmaskColor.r / 255.0f;
            }

            if (!lightingFunction)
            {
                continue;
            }

            // Create light emitter / absorber for tile
            for (int subX = 0; subX < TILE_LIGHTING_RESOLUTION; subX++)
            {
                for (int subY = 0; subY < TILE_LIGHTING_RESOLUTION; subY++)
                {
                    (lightingEngine.*lightingFunction)(lightingTileX + x * TILE_LIGHTING_RESOLUTION + subX, lightingTileY + y * TILE_LIGHTING_RESOLUTION + subY, lightingValue);
                }
            }
        }
    }
}

StructureObjectPOD StructureObject::getPOD(pl::Vector2f chunkPosition)
{
    StructureObjectPOD pod;
    pod.structureType = structureType;
    pod.structureID = structureID;
    pod.relativePos = position - chunkPosition;
    pod.warpEntranceRectRelative = warpEntranceRect;
    if (warpEntranceRect.has_value())
    {
        pod.warpEntranceRectRelative->x -= chunkPosition.x;
        pod.warpEntranceRectRelative->y -= chunkPosition.y;
    }
    return pod;
}

void StructureObject::loadFromPOD(const StructureObjectPOD& pod, pl::Vector2f chunkPosition)
{
    structureType = pod.structureType;
    structureID = pod.structureID;
    position = chunkPosition + pod.relativePos;
    warpEntranceRect = pod.warpEntranceRectRelative;
    if (warpEntranceRect.has_value())
    {
        warpEntranceRect->x += chunkPosition.x;
        warpEntranceRect->y += chunkPosition.y;
    }
}