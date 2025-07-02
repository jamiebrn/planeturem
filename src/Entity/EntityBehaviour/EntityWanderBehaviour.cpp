#include "Entity/EntityBehaviour/EntityWanderBehaviour.hpp"
#include "Entity/Entity.hpp"
#include "Game.hpp"

EntityWanderBehaviour::EntityWanderBehaviour(Entity& entity)
{
    float velocityAngle = rand() % 360;
    pl::Vector2f velocity;
    velocity.x = std::cos(velocityAngle * 2 * 3.14 / 180) * 23.0f;
    velocity.y = std::sin(velocityAngle * 2 * 3.14 / 180) * 23.0f;

    velocityMult = 1.0f;

    entity.setVelocity(velocity);
}

void EntityWanderBehaviour::update(Entity& entity, ChunkManager& chunkManager, Game& game, float dt)
{
    CollisionRect collisionRect = entity.getCollisionRect();
    pl::Vector2f velocity = entity.getVelocity();

    // Test collision after x movement
    collisionRect.x += velocity.x * velocityMult * dt;
    if (chunkManager.collisionRectChunkStaticCollisionX(collisionRect, velocity.x))
        velocity.x *= -1;

    // Test collision after y movement
    collisionRect.y += velocity.y * velocityMult * dt;
    if (chunkManager.collisionRectChunkStaticCollisionY(collisionRect, velocity.y))
        velocity.y *= -1;
    
    velocityMult = Helper::lerp(velocityMult, 1.0f, VELOCITY_MULT_LERP_WEIGHT * dt);
    
    entity.setCollisionRect(collisionRect);
    entity.setVelocity(velocity);
    entity.setAnimationSpeed(1.0f + (velocityMult - 1.0f) * 0.4f);
}

void EntityWanderBehaviour::onHit(Entity& entity, Game& game, const LocationState& locationState, pl::Vector2f hitSource)
{
    pl::Vector2f relativePos = hitSource - entity.getPosition();

    pl::Vector2f oldVelocity = entity.getVelocity();

    pl::Vector2f velocity = -Helper::normaliseVector(relativePos) * Helper::getVectorLength(oldVelocity);

    velocityMult = 2.2f;
    
    entity.setVelocity(velocity);
}