#include "Entity/EntityBehaviour/EntityRabbitBehaviour.hpp"
#include "Entity/Entity.hpp"
#include "Game.hpp"

EntityRabbitBehaviour::EntityRabbitBehaviour(Entity& entity)
{
    float velocityAngle = rand() % 360;
    pl::Vector2f velocity;
    velocity.x = std::cos(velocityAngle * 2 * 3.14 / 180) * 23.0f;
    velocity.y = std::sin(velocityAngle * 2 * 3.14 / 180) * 23.0f;

    velocityMult = 1.0f;

    entity.setVelocity(velocity);

    idleWaitTime = Helper::randFloat(0.05f, MAX_IDLE_WAIT_TIME);

    const EntityData& entityData = EntityDataLoader::getEntityData(entity.getEntityType());

    if (entityData.behaviourParameters.contains("walk-speed"))
    {
        walkSpeed = entityData.behaviourParameters.at("walk-speed");
    }
}

void EntityRabbitBehaviour::update(Entity& entity, ChunkManager& chunkManager, Game& game, float dt)
{
    bool idleWaitTimeWasAboveZero = idleWaitTime > 0.0f;

    idleWaitTime -= dt;

    if (idleWaitTime > 0.0f)
    {
        entity.setVelocity(pl::Vector2f(0, 0));
        return;
    }
    else
    {
        // Wait time over - walk to new location
        if (idleWaitTimeWasAboveZero)
        {
            targetPosition = entity.getPosition() + pl::Vector2f(Helper::randFloat(MIN_TARGET_RANGE, MAX_TARGET_RANGE), 0).rotate(Helper::randFloat(0, 2 * M_PI));
            entity.setVelocity((Camera::translateWorldPos(targetPosition, entity.getPosition(), chunkManager.getWorldSize()) - entity.getPosition()).normalise() * walkSpeed);
            entity.setIdleAnimationFrame(0);
            entity.setWalkAnimationFrame(0);
        }
    }

    CollisionRect collisionRect = entity.getCollisionRect();
    pl::Vector2f velocity = entity.getVelocity();

    bool stopWalking = false;

    // Test collision after x movement
    collisionRect.x += velocity.x * velocityMult * dt;
    if (chunkManager.collisionRectChunkStaticCollisionX(collisionRect, velocity.x))
    {
        stopWalking = true;
    }

    // Test collision after y movement
    collisionRect.y += velocity.y * velocityMult * dt;
    if (chunkManager.collisionRectChunkStaticCollisionY(collisionRect, velocity.y))
    {
        stopWalking = true;
    }

    float distanceToTarget = (entity.getPosition() - Camera::translateWorldPos(targetPosition, entity.getPosition(), chunkManager.getWorldSize())).getLength();
    if (distanceToTarget <= TARGET_RANGE_REACH_THRESHOLD)
    {
        stopWalking = true;
    }

    if (stopWalking)
    {
        velocity = pl::Vector2f(0, 0);
        idleWaitTime = Helper::randFloat(MIN_IDLE_WAIT_TIME, MAX_IDLE_WAIT_TIME);
    }
    
    velocityMult = Helper::lerp(velocityMult, 1.0f, VELOCITY_MULT_LERP_WEIGHT * dt);
    
    entity.setCollisionRect(collisionRect);
    entity.setVelocity(velocity);
    entity.setAnimationSpeed(1.0f + (velocityMult - 1.0f) * 0.4f);
}

void EntityRabbitBehaviour::onHit(Entity& entity, Game& game, pl::Vector2f hitSource)
{
    pl::Vector2f relativePos = hitSource - entity.getPosition();

    pl::Vector2f oldVelocity = entity.getVelocity();

    pl::Vector2f velocity = -Helper::normaliseVector(relativePos) * Helper::getVectorLength(oldVelocity);

    velocityMult = 2.2f;
    
    entity.setVelocity(velocity);
}