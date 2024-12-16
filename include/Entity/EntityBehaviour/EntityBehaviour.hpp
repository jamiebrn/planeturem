#pragma once

#include <SFML/Graphics.hpp>

class ChunkManager;
class Entity;

class EntityBehaviour
{
public:
    EntityBehaviour() = default;

    virtual void update(Entity& entity, ChunkManager& chunkManager, float dt) = 0;
};