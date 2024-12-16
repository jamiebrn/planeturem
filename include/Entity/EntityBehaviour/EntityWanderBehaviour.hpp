#pragma once

#include <SFML/Graphics.hpp>

#include "Entity/EntityBehaviour/EntityBehaviour.hpp"

class EntityWanderBehaviour : public EntityBehaviour
{
public:
    EntityWanderBehaviour(Entity& entity);

    void update(Entity& entity, ChunkManager& chunkManager, float dt) override;
};