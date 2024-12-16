#include "Entity/EntityBehaviour/EntityWanderBehaviour.hpp"
#include "Entity/Entity.hpp"

EntityWanderBehaviour::EntityWanderBehaviour(Entity& entity)
{
    float velocityAngle = rand() % 360;
    sf::Vector2f velocity;
    velocity.x = std::cos(velocityAngle * 2 * 3.14 / 180) * 23.0f;
    velocity.y = std::sin(velocityAngle * 2 * 3.14 / 180) * 23.0f;

    entity.setVelocity(velocity);
}

void EntityWanderBehaviour::update(Entity& entity, ChunkManager& chunkManager, float dt)
{
    CollisionRect collisionRect = entity.getCollisionRect();
    sf::Vector2f velocity = entity.getVelocity();

    // Test collision after x movement
    collisionRect.x += velocity.x * dt;
    if (chunkManager.collisionRectChunkStaticCollisionX(collisionRect, velocity.x))
        velocity.x *= -1;

    // Test collision after y movement
    collisionRect.y += velocity.y * dt;
    if (chunkManager.collisionRectChunkStaticCollisionY(collisionRect, velocity.y))
        velocity.y *= -1;
    
    entity.setCollisionRect(collisionRect);
    entity.setVelocity(velocity);
}