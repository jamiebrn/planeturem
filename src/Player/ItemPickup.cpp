#include "Player/ItemPickup.hpp"

bool ItemPickup::isBeingPickedUp(const CollisionRect& playerCollision, float gameTime) const
{
    if (gameTime - spawnGameTime < SPAWN_FLASH_TIME)
    {
        return false;
    }

    CollisionCircle collisionCircle(position.x, position.y, PICKUP_RADIUS);
    return playerCollision.isColliding(collisionCircle);
}

void ItemPickup::resetSpawnTime(float gameTime)
{
    spawnGameTime = gameTime;
}

void ItemPickup::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const pl::Color& color) const
{
    static const pl::Rect<int> shadowRect(0, 496, 16, 16);
    static constexpr float maxFloatHeight = 10.0f;
    static constexpr float minFloatHeight = 6.0f;

    spriteBatch.endDrawing(window);

    float scale = ResolutionHandler::getScale();

    pl::DrawData shadowDrawData;
    shadowDrawData.texture = TextureManager::getTexture(TextureType::Items);
    shadowDrawData.shader = Shaders::getShader(ShaderType::Default);
    shadowDrawData.position = camera.worldToScreenTransform(position);
    shadowDrawData.scale = pl::Vector2f(scale, scale);
    shadowDrawData.centerRatio = pl::Vector2f(0.5f, 0.5f);
    shadowDrawData.textureRect = shadowRect;

    TextureManager::drawSubTexture(window, shadowDrawData);

    pl::Vector2<int> chunkTile = getChunkTileInside(worldSize);
    float floatAmount = std::pow(std::sin(gameTime + chunkTile.x + chunkTile.y), 2);

    pl::Vector2f screenPos = camera.worldToScreenTransform(position - pl::Vector2f(0, floatAmount * (maxFloatHeight - minFloatHeight) + minFloatHeight));
    float scaleMult = scale / 3.0f / ResolutionHandler::getResolutionIntegerScale();

    float flashAmount = std::max(SPAWN_FLASH_TIME - (gameTime - spawnGameTime), 0.0f) / SPAWN_FLASH_TIME;

    ItemSlot::drawItem(window, spriteBatch, itemType, screenPos, scaleMult, true, 255, flashAmount);

    if (count > 1)
    {
        spriteBatch.endDrawing(window);

        float intScale = ResolutionHandler::getResolutionIntegerScale();

        TextDraw::drawText(window, {
            std::to_string(count),
            screenPos + pl::Vector2f(24 / 4.0f, 24 / 4.0f) * scale,
            {255, 255, 255},
            24 * static_cast<unsigned int>(intScale),
            pl::Color(46, 34, 47),
            2 * static_cast<unsigned int>(intScale),
            true,
            true});
    }
}