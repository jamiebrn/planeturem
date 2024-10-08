#include "Entity/Entity.hpp"

Entity::Entity(sf::Vector2f position, EntityType entityType)
    : WorldObject(position)
{
    this->entityType = entityType;

    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);

    health = entityData.health;

    float velocityAngle = rand() % 360;
    velocity.x = std::cos(velocityAngle * 2 * 3.14 / 180) * 23.0f;
    velocity.y = std::sin(velocityAngle * 2 * 3.14 / 180) * 23.0f;

    collisionRect.width = TILE_SIZE_PIXELS_UNSCALED * entityData.size.x;
    collisionRect.height = TILE_SIZE_PIXELS_UNSCALED * entityData.size.y;

    collisionRect.x = position.x - collisionRect.width / 2.0f;
    collisionRect.y = position.y - collisionRect.height / 2.0f;

    drawLayer = 0;

    flash_amount = 0.0f;
    
    idleAnim.setFrame(rand() % entityData.idleTextureRects.size());
    walkAnim.setFrame(rand() % entityData.walkTextureRects.size());
}

void Entity::update(float dt, ChunkManager& chunkManager, bool onWater)
{
    // Handle collision with world (tiles, object)

    // Test collision after x movement
    collisionRect.x += velocity.x * dt;
    if (chunkManager.collisionRectChunkStaticCollisionX(collisionRect, velocity.x))
        velocity.x *= -1;

    // Test collision after y movement
    collisionRect.y += velocity.y * dt;
    if (chunkManager.collisionRectChunkStaticCollisionY(collisionRect, velocity.y))
        velocity.y *= -1;

    // Update position using collision rect after collision has been handled
    position.x = collisionRect.x + collisionRect.width / 2.0f;
    position.y = collisionRect.y + collisionRect.height / 2.0f;

    // Update animations
    flash_amount = std::max(flash_amount - dt * 3.0f, 0.0f);

    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);
    idleAnim.update(dt, 1, entityData.idleTextureRects.size(), entityData.idleAnimSpeed);
    walkAnim.update(dt, 1, entityData.walkTextureRects.size(), entityData.walkAnimSpeed);

    this->onWater = onWater;
}

void Entity::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) const
{
    spriteBatch.endDrawing(window);

    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);

    sf::Vector2f scale(ResolutionHandler::getScale(), ResolutionHandler::getScale());

    float waterYOffset = getWaterBobYOffset(worldSize, gameTime);

    // Draw shadow
    TextureManager::drawTexture(window, {TextureType::Shadow, Camera::worldToScreenTransform(position + sf::Vector2f(0, waterYOffset)), 0, scale, {0.5, 0.85}});

    if (velocity.x < 0)
        scale.x *= -1;
    
    sf::Shader* shader = Shaders::getShader(ShaderType::Flash);
    shader->setUniform("flash_amount", flash_amount);

    sf::IntRect textureRect;
    if (velocity.x == 0 && velocity.y == 0)
    {
        textureRect = entityData.idleTextureRects[idleAnim.getFrame()];
    }
    else
    {
        textureRect = entityData.walkTextureRects[walkAnim.getFrame()];
    }

    TextureManager::drawSubTexture(window, {TextureType::Entities, Camera::worldToScreenTransform(position + sf::Vector2f(0, waterYOffset)), 0, 
        scale, entityData.textureOrigin, color}, textureRect, shader);

    // DEBUG
    if (DebugOptions::drawCollisionRects)
    {
        collisionRect.debugDraw(window);
    }
}

void Entity::drawLightMask(sf::RenderTarget& lightTexture)
{
    static constexpr float lightScale = 0.3f;
    static const sf::Color lightColor(255, 220, 140);

    sf::Vector2f scale((float)ResolutionHandler::getScale() * lightScale, (float)ResolutionHandler::getScale() * lightScale);

    sf::IntRect lightMaskRect(0, 0, 256, 256);

    TextureManager::drawSubTexture(lightTexture, {
        TextureType::LightMask, Camera::worldToScreenTransform(position), 0, scale, {0.5, 0.5}, lightColor
        }, lightMaskRect, sf::BlendAdd);
}


void Entity::damage(int amount, InventoryData& inventory)
{
    flash_amount = 1.0f;
    health -= amount;

    SoundType hitSound = SoundType::HitObject;
    int soundChance = rand() % 3;
    if (soundChance == 1) hitSound = SoundType::HitObject2;
    else if (soundChance == 2) hitSound = SoundType::HitObject3;

    Sounds::playSound(hitSound, 60.0f);

    if (!isAlive())
    {
        // Give item drops
        const EntityData& entityData = EntityDataLoader::getEntityData(entityType);
        float dropChance = (rand() % 1000) / 1000.0f;
        for (const ItemDrop& itemDrop : entityData.itemDrops)
        {
            if (dropChance < itemDrop.chance)
            {
                // Give items
                unsigned int itemAmount = rand() % std::max(itemDrop.maxAmount - itemDrop.minAmount + 1, 1U) + itemDrop.minAmount;
                if (itemAmount <= 0)
                    continue;

                inventory.addItem(itemDrop.item, itemAmount);

                // Create item popup
                InventoryGUI::pushItemPopup(ItemCount(itemDrop.item, itemAmount));
            }
        }
    }
}

void Entity::interact()
{

}

bool Entity::isSelectedWithCursor(sf::Vector2f cursorWorldPos)
{
    return collisionRect.isPointInRect(cursorWorldPos.x, cursorWorldPos.y);
}

void Entity::setWorldPosition(sf::Vector2f newPosition)
{
    collisionRect.x = newPosition.x - collisionRect.width / 2.0f;
    collisionRect.y = newPosition.y - collisionRect.height / 2.0f;
}

EntityType Entity::getEntityType()
{
    return entityType;
}

sf::Vector2f Entity::getSize()
{
    return sf::Vector2f(collisionRect.width, collisionRect.height);
}

const CollisionRect& Entity::getCollisionRect()
{
    return collisionRect;
}

EntityPOD Entity::getPOD(sf::Vector2f chunkPosition)
{
    EntityPOD pod;
    pod.entityType = entityType;
    pod.chunkRelativePosition = position - chunkPosition;
    pod.velocity = velocity;
    return pod;
}

void Entity::loadFromPOD(const EntityPOD& pod, sf::Vector2f chunkPosition)
{
    entityType = pod.entityType;
    position = pod.chunkRelativePosition + chunkPosition;
    velocity = pod.velocity;
}