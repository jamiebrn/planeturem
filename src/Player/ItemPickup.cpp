#include "Player/ItemPickup.hpp"

bool ItemPickup::isBeingPickedUp(const CollisionRect& playerCollision, float gameTime)
{
    if (gameTime - spawnGameTime < SPAWN_FLASH_TIME)
    {
        return false;
    }

    CollisionCircle collisionCircle(position.x, position.y, PICKUP_RADIUS);
    return playerCollision.isColliding(collisionCircle);
}

void ItemPickup::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const sf::Color& color) const
{
    static const sf::IntRect shadowRect(0, 496, 16, 16);
    static constexpr float maxFloatHeight = 10.0f;
    static constexpr float minFloatHeight = 6.0f;

    spriteBatch.endDrawing(window);

    float scale = ResolutionHandler::getScale();

    sf::Vector2f shadowScreenPos = camera.worldToScreenTransform(position);

    TextureDrawData shadowDrawData;
    shadowDrawData.type = TextureType::Items;
    shadowDrawData.position = shadowScreenPos;
    shadowDrawData.scale = sf::Vector2f(scale, scale);
    shadowDrawData.centerRatio = sf::Vector2f(0.5f, 0.5f);
    TextureManager::drawSubTexture(window, shadowDrawData, shadowRect);

    sf::Vector2i worldTile = getWorldTileInside(worldSize);
    float floatAmount = std::pow(std::sin(gameTime + worldTile.x + worldTile.y), 2);

    sf::Vector2f screenPos = camera.worldToScreenTransform(position - sf::Vector2f(0, floatAmount * (maxFloatHeight - minFloatHeight) + minFloatHeight));
    float scaleMult = scale / 3.0f;

    float flashAmount = std::max(SPAWN_FLASH_TIME - (gameTime - spawnGameTime), 0.0f) / SPAWN_FLASH_TIME;

    ItemSlot::drawItem(window, itemType, screenPos, scaleMult, true, 255, flashAmount);
}