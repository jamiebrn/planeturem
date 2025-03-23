#include "Entity/Entity.hpp"
#include "Game.hpp"
#include "World/ChunkManager.hpp"

#include "Entity/EntityBehaviour/EntityWanderBehaviour.hpp"
#include "Entity/EntityBehaviour/EntityFollowAttackBehaviour.hpp"

Entity::Entity(sf::Vector2f position, EntityType entityType)
    : WorldObject(position)
{
    this->entityType = entityType;

    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);

    health = entityData.health;

    initialiseBehaviour(entityData.behaviour);

    collisionRect.width = TILE_SIZE_PIXELS_UNSCALED * entityData.size.x;
    collisionRect.height = TILE_SIZE_PIXELS_UNSCALED * entityData.size.y;

    collisionRect.x = position.x - collisionRect.width / 2.0f;
    collisionRect.y = position.y - collisionRect.height / 2.0f;

    drawLayer = 0;

    flashAmount = 0.0f;
    
    idleAnim.setFrame(rand() % entityData.idleTextureRects.size());
    walkAnim.setFrame(rand() % entityData.walkTextureRects.size());
}

void Entity::initialiseBehaviour(const std::string& behaviour)
{
    if (behaviour == "wander")
    {
        this->behaviour = std::make_unique<EntityWanderBehaviour>(*this);
    }
    else if (behaviour == "followattack")
    {
        this->behaviour = std::make_unique<EntityFollowAttackBehaviour>(*this);
    }
}

void Entity::update(float dt, ProjectileManager& projectileManager, ChunkManager& chunkManager, Game& game, bool onWater, float gameTime)
{
    if (behaviour)
    {
        behaviour->update(*this, chunkManager, game, dt);
    }

    // Update position using collision rect after collision has been handled
    position.x = collisionRect.x + collisionRect.width / 2.0f;
    position.y = collisionRect.y + collisionRect.height / 2.0f;

    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);
    hitCollision = entityData.hitCollision;
    hitCollision.x += position.x;
    hitCollision.y += position.y;

    // Test collision with projectiles
    for (auto& projectilePair : projectileManager.getProjectiles())
    {
        if (isProjectileColliding(projectilePair.second) && projectilePair.second.isAlive())
        {
            damage(projectilePair.second.getDamage(), chunkManager, gameTime);
            projectilePair.second.onCollision();
            behaviour->onHit(*this, game, projectilePair.second.getPosition());
        }
    }

    // Update animations
    flashAmount = std::max(flashAmount - dt * 3.0f, 0.0f);

    idleAnim.update(dt * animationSpeed, 1, entityData.idleTextureRects.size(), entityData.idleAnimSpeed);
    walkAnim.update(dt * animationSpeed, 1, entityData.walkTextureRects.size(), entityData.walkAnimSpeed);

    this->onWater = onWater;
}

void Entity::updateNetwork(float dt, ChunkManager& chunkManager)
{
    // Test collision after x movement
    collisionRect.x += velocity.x * dt;
    chunkManager.collisionRectChunkStaticCollisionX(collisionRect, velocity.x);

    // Test collision after y movement
    collisionRect.y += velocity.y * dt;
    chunkManager.collisionRectChunkStaticCollisionY(collisionRect, velocity.y);

    position.x = collisionRect.x + collisionRect.width / 2.0f;
    position.y = collisionRect.y + collisionRect.height / 2.0f;

    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);

    // Update animations
    flashAmount = std::max(flashAmount - dt * 3.0f, 0.0f);

    idleAnim.update(dt * animationSpeed, 1, entityData.idleTextureRects.size(), entityData.idleAnimSpeed);
    walkAnim.update(dt * animationSpeed, 1, entityData.walkTextureRects.size(), entityData.walkAnimSpeed);
}

bool Entity::isProjectileColliding(Projectile& projectile)
{
    return (hitCollision.isPointInRect(projectile.getPosition().x, projectile.getPosition().y));
}

void Entity::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const sf::Color& color) const
{
    spriteBatch.endDrawing(window);

    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);

    sf::Vector2f scale(ResolutionHandler::getScale(), ResolutionHandler::getScale());

    float waterYOffset = getWaterBobYOffset(worldSize, gameTime);

    // Draw shadow
    TextureManager::drawTexture(window, {TextureType::Shadow, camera.worldToScreenTransform(position + sf::Vector2f(0, waterYOffset)), 0, scale, {0.5, 0.85}});

    if (velocity.x < 0)
        scale.x *= -1;
    
    sf::Shader* shader = Shaders::getShader(ShaderType::Flash);
    shader->setUniform("flash_amount", flashAmount);

    sf::IntRect textureRect;
    if (velocity.x == 0 && velocity.y == 0)
    {
        textureRect = entityData.idleTextureRects[idleAnim.getFrame()];
    }
    else
    {
        textureRect = entityData.walkTextureRects[walkAnim.getFrame()];
    }

    TextureManager::drawSubTexture(window, {TextureType::Entities, camera.worldToScreenTransform(position + sf::Vector2f(0, waterYOffset)), 0, 
        scale, entityData.textureOrigin, color}, textureRect, shader);

    #if (!RELEASE_BUILD)
    // DEBUG
    if (DebugOptions::drawCollisionRects)
    {
        collisionRect.debugDraw(window, game.getCamera());
        hitCollision.debugDraw(window, game.getCamera());
    }
    #endif
}

void Entity::createLightSource(LightingEngine& lightingEngine, sf::Vector2f topLeftChunkPos) const
{
    // static constexpr float lightScale = 0.3f;
    // static const sf::Color lightColor(255, 220, 140);

    // sf::Vector2f scale((float)ResolutionHandler::getScale() * lightScale, (float)ResolutionHandler::getScale() * lightScale);

    // sf::IntRect lightMaskRect(0, 0, 256, 256);

    // TextureManager::drawSubTexture(lightTexture, {
    //     TextureType::LightMask, Camera::worldToScreenTransform(position), 0, scale, {0.5, 0.5}, lightColor
    //     }, lightMaskRect, sf::BlendAdd);
}


void Entity::testHitCollision(const std::vector<HitRect>& hitRects, ChunkManager& chunkManager, Game& game, float gameTime)
{
    for (const HitRect& hitRect : hitRects)
    {
        if (hitRect.isColliding(collisionRect))
        {
            damage(hitRect.damage, chunkManager, gameTime);
            behaviour->onHit(*this, game, game.getPlayer().getPosition());
            return;
        }
    }
}

void Entity::damage(int amount, ChunkManager& chunkManager, float gameTime)
{
    flashAmount = 1.0f;
    health -= amount;

    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);

    HitMarkers::addHitMarker(position, amount);

    // SoundType hitSound = SoundType::HitObject;
    // int soundChance = rand() % 3;
    // if (soundChance == 1) hitSound = SoundType::HitObject2;
    // else if (soundChance == 2) hitSound = SoundType::HitObject3;

    // Sounds::playSound(hitSound, 60.0f);

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

                for (int i = 0; i < itemAmount; i++)
                {
                    sf::Vector2f spawnPos = position - sf::Vector2f(0.5f, 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
                    spawnPos.x += Helper::randFloat(0.0f, entityData.size.x * TILE_SIZE_PIXELS_UNSCALED);
                    spawnPos.y += Helper::randFloat(0.0f, entityData.size.y * TILE_SIZE_PIXELS_UNSCALED);

                    chunkManager.addItemPickup(ItemPickup(spawnPos, itemDrop.item, gameTime));
                }

                // inventory.addItem(itemDrop.item, itemAmount, true);
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

void Entity::setCollisionRect(const CollisionRect& rect)
{
    collisionRect = rect;
}

sf::Vector2f Entity::getVelocity()
{
    return velocity;
}

void Entity::setVelocity(sf::Vector2f velocity)
{
    this->velocity = velocity;
}

void Entity::setAnimationSpeed(float speed)
{
    animationSpeed = speed;
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
    collisionRect.x = position.x - collisionRect.width / 2.0f;
    collisionRect.y = position.y - collisionRect.height / 2.0f;
    velocity = pod.velocity;
}

PacketDataEntities::EntityPacketData Entity::getPacketData(sf::Vector2f chunkPosition)
{
    EntityPOD pod = getPOD(chunkPosition);
    
    PacketDataEntities::EntityPacketData packetData;
    packetData.entityType = pod.entityType;
    packetData.chunkRelativePositionX = CompactFloat<uint16_t>(pod.chunkRelativePosition.x, 2);
    packetData.chunkRelativePositionY = CompactFloat<uint16_t>(pod.chunkRelativePosition.y, 2);
    packetData.velocityX = CompactFloat<uint16_t>(pod.chunkRelativePosition.x, 2);
    packetData.velocityY = CompactFloat<uint16_t>(pod.chunkRelativePosition.y, 2);
    packetData.health = health;
    packetData.flashAmount = CompactFloat<uint8_t>(flashAmount, 2);
    packetData.idleAnimFrame = idleAnim.getFrame();
    packetData.walkAnimFrame = walkAnim.getFrame();

    return packetData;
}

void Entity::loadFromPacketData(const PacketDataEntities::EntityPacketData& packetData, sf::Vector2f chunkPosition)
{
    EntityPOD pod;
    pod.entityType = packetData.entityType;
    pod.chunkRelativePosition.x = packetData.chunkRelativePositionX.getValue(2);
    pod.chunkRelativePosition.y = packetData.chunkRelativePositionY.getValue(2);
    pod.velocity.x = packetData.velocityX.getValue(2);
    pod.velocity.y = packetData.velocityY.getValue(2);
    loadFromPOD(pod, chunkPosition);

    health = packetData.health;
    flashAmount = packetData.flashAmount.getValue(2);
    idleAnim.setFrame(packetData.idleAnimFrame);
    walkAnim.setFrame(packetData.walkAnimFrame);
}