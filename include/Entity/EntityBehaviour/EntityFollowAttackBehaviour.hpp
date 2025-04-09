#pragma once

// #include <SFML/Graphics.hpp>

#include <Vector.hpp>
#include <Rect.hpp>

#include "World/PathfindingEngine.hpp"

#include "Entity/EntityBehaviour/EntityBehaviour.hpp"

class EntityFollowAttackBehaviour : public EntityBehaviour
{
public:
    EntityFollowAttackBehaviour(Entity& entity);

    void update(Entity& entity, ChunkManager& chunkManager, Game& game, float dt) override;

private:
    PathFollower pathFollower;
    bool collisionLastFrame;

};