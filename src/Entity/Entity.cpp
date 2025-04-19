#include "Entity/Entity.hpp"
#include "Game.hpp"
#include "World/ChunkManager.hpp"

#include "Entity/EntityBehaviour/EntityWanderBehaviour.hpp"
#include "Entity/EntityBehaviour/EntityFollowAttackBehaviour.hpp"

Entity::Entity(pl::Vector2f position, EntityType entityType)
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
            damage(projectilePair.second.getDamage(), game, LocationState::createFromPlanetType(chunkManager.getPlanetType()), gameTime);
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

void Entity::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const pl::Color& color) const
{
    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);

    pl::Vector2f scale(ResolutionHandler::getScale(), ResolutionHandler::getScale());

    float waterYOffset = getWaterBobYOffset(worldSize, gameTime);

    // Draw shadow
    pl::DrawData shadowDrawData;
    shadowDrawData.texture = TextureManager::getTexture(TextureType::Shadow);
    shadowDrawData.shader = Shaders::getShader(ShaderType::Default);
    shadowDrawData.position = camera.worldToScreenTransform(position + pl::Vector2f(0, waterYOffset));
    shadowDrawData.scale = scale;
    shadowDrawData.centerRatio = pl::Vector2f(0.5, 0.85);
    shadowDrawData.textureRect = pl::Rect<int>(0, 0, shadowDrawData.texture->getWidth(), shadowDrawData.texture->getHeight());

    spriteBatch.draw(window, shadowDrawData);

    if (velocity.x < 0)
    {
        scale.x *= -1;
    }

    pl::DrawData entityDrawData;
    entityDrawData.texture = TextureManager::getTexture(TextureType::Entities);
    entityDrawData.shader = Shaders::getShader(ShaderType::Default);
    entityDrawData.position = shadowDrawData.position;
    entityDrawData.scale = scale;
    entityDrawData.centerRatio = entityData.textureOrigin;
    entityDrawData.color = color;

    if (flashAmount > 0)
    {
        entityDrawData.shader = Shaders::getShader(ShaderType::Flash);
        entityDrawData.shader->setUniform1f("flash_amount", flashAmount);
    }
    
    if (velocity.x == 0 && velocity.y == 0)
    {
        entityDrawData.textureRect = entityData.idleTextureRects[idleAnim.getFrame()];
    }
    else
    {
        entityDrawData.textureRect = entityData.walkTextureRects[walkAnim.getFrame()];
    }

    spriteBatch.draw(window, entityDrawData);

    #if (!RELEASE_BUILD)
    // DEBUG
    if (DebugOptions::drawCollisionRects)
    {
        collisionRect.debugDraw(window, game.getCamera());
        hitCollision.debugDraw(window, game.getCamera());
    }
    #endif
}

void Entity::createLightSource(LightingEngine& lightingEngine, pl::Vector2f topLeftChunkPos) const
{
    // static constexpr float lightScale = 0.3f;
    // static const sf::Color lightColor(255, 220, 140);

    // pl::Vector2f scale((float)ResolutionHandler::getScale() * lightScale, (float)ResolutionHandler::getScale() * lightScale);

    // pl::Rect<int> lightMaskRect(0, 0, 256, 256);

    // TextureManager::drawSubTexture(lightTexture, {
    //     TextureType::LightMask, Camera::worldToScreenTransform(position), 0, scale, {0.5, 0.5}, lightColor
    //     }, lightMaskRect, sf::BlendAdd);
}


void Entity::testHitCollision(const std::vector<HitRect>& hitRects, Game& game, const LocationState& locationState, float gameTime)
{
    for (const HitRect& hitRect : hitRects)
    {
        if (hitRect.isColliding(collisionRect))
        {
            damage(hitRect.damage, game, locationState, gameTime);
            behaviour->onHit(*this, game, game.getPlayer().getPosition());
            return;
        }
    }
}

void Entity::damage(int amount, Game& game, const LocationState& locationState, float gameTime)
{
    flashAmount = 1.0f;
    health -= amount;

    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);

    HitMarkers::addHitMarker(position, amount);

    SoundType hitSound = SoundType::HitAnimal;
    int soundChance = rand() % 3;
    if (soundChance == 1) hitSound = SoundType::HitAnimal2;
    else if (soundChance == 2) hitSound = SoundType::HitAnimal3;

    Sounds::playSound(hitSound, 30.0f);

    if (!isAlive())
    {
        // Get chunk manager
        if (!game.isLocationStateInitialised(locationState))
        {
            printf("ERROR: Attempted to create item pickups for entity in null location\n");
            return;
        }

        ChunkManager& chunkManager = game.getChunkManager(locationState.getPlanetType());

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
                    pl::Vector2f spawnPos = position - pl::Vector2f(0.5f, 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
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

bool Entity::isSelectedWithCursor(pl::Vector2f cursorWorldPos)
{
    return collisionRect.isPointInRect(cursorWorldPos.x, cursorWorldPos.y);
}

void Entity::setWorldPosition(pl::Vector2f newPosition)
{
    collisionRect.x = newPosition.x - collisionRect.width / 2.0f;
    collisionRect.y = newPosition.y - collisionRect.height / 2.0f;
}

EntityType Entity::getEntityType()
{
    return entityType;
}

pl::Vector2f Entity::getSize()
{
    return pl::Vector2f(collisionRect.width, collisionRect.height);
}

const CollisionRect& Entity::getCollisionRect()
{
    return collisionRect;
}

void Entity::setCollisionRect(const CollisionRect& rect)
{
    collisionRect = rect;
}

pl::Vector2f Entity::getVelocity()
{
    return velocity;
}

void Entity::setVelocity(pl::Vector2f velocity)
{
    this->velocity = velocity;
}

void Entity::setAnimationSpeed(float speed)
{
    animationSpeed = speed;
}

EntityPOD Entity::getPOD(pl::Vector2f chunkPosition)
{
    EntityPOD pod;
    pod.entityType = entityType;
    pod.chunkRelativePosition = position - chunkPosition;
    pod.velocity = velocity;
    return pod;
}

void Entity::loadFromPOD(const EntityPOD& pod, pl::Vector2f chunkPosition)
{
    entityType = pod.entityType;
    position = pod.chunkRelativePosition + chunkPosition;
    collisionRect.x = position.x - collisionRect.width / 2.0f;
    collisionRect.y = position.y - collisionRect.height / 2.0f;
    velocity = pod.velocity;
}

PacketDataEntities::EntityPacketData Entity::getPacketData(pl::Vector2f chunkPosition)
{
    EntityPOD pod = getPOD(chunkPosition);
    
    PacketDataEntities::EntityPacketData packetData;
    packetData.entityType = pod.entityType;
    packetData.chunkRelativePositionX = CompactFloat<uint16_t>(pod.chunkRelativePosition.x, 2);
    packetData.chunkRelativePositionY = CompactFloat<uint16_t>(pod.chunkRelativePosition.y, 2);
    packetData.velocityX = CompactFloat<int16_t>(pod.velocity.x, 2);
    packetData.velocityY = CompactFloat<int16_t>(pod.velocity.y, 2);
    packetData.health = health;
    packetData.flashAmount = CompactFloat<uint8_t>(flashAmount, 2);
    packetData.idleAnimFrame = idleAnim.getFrame();
    packetData.walkAnimFrame = walkAnim.getFrame();

    return packetData;
}

void Entity::loadFromPacketData(const PacketDataEntities::EntityPacketData& packetData, pl::Vector2f chunkPosition)
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