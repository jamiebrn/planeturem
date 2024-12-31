#include "Entity/EntityBehaviour/EntityFollowAttackBehaviour.hpp"
#include "Entity/Entity.hpp"
#include "Game.hpp"

EntityFollowAttackBehaviour::EntityFollowAttackBehaviour(Entity& entity)
{
    collisionLastFrame = false;
}

void EntityFollowAttackBehaviour::update(Entity& entity, ChunkManager& chunkManager, Game& game, float dt)
{
    CollisionRect collisionRect = entity.getCollisionRect();

    if (!pathFollower.isActive() || collisionLastFrame)
    {
        sf::Vector2i tile = WorldObject::getWorldTileInside(sf::Vector2f(collisionRect.x, collisionRect.y), chunkManager.getWorldSize());
        sf::Vector2i playerTile = game.getPlayer().getWorldTileInside(chunkManager.getWorldSize());

        std::vector<PathfindGridCoordinate> pathfindResult;
        if (chunkManager.getPathfindingEngine().findPath(tile.x, tile.y, playerTile.x, playerTile.y, pathfindResult, true, 70))
        {
            pathFollower.beginPath(sf::Vector2f(collisionRect.x, collisionRect.y), chunkManager.getPathfindingEngine().createStepSequenceFromPath(pathfindResult));
        }

        collisionLastFrame = false;
    }
    else
    {
        // TODO: Replace with custom speed
        sf::Vector2f pathPos = pathFollower.updateFollower(60.0f * dt);
        sf::Vector2f velocity = (pathPos - sf::Vector2f(collisionRect.x, collisionRect.y)) / dt;
        entity.setVelocity(velocity);

        // Test collision after x movement
        collisionRect.x = pathPos.x;
        if (chunkManager.collisionRectChunkStaticCollisionX(collisionRect, velocity.x))
        {
            collisionLastFrame = true;
        }

        // Test collision after y movement
        collisionRect.y = pathPos.y;
        if (chunkManager.collisionRectChunkStaticCollisionY(collisionRect, velocity.y))
        {
            collisionLastFrame = true;
        }
    }

    entity.setCollisionRect(collisionRect);
}