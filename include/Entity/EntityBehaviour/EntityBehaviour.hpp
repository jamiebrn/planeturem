#pragma once

#include <Vector.hpp>
#include <Rect.hpp>

#include "Player/LocationState.hpp"

class Game;
class Entity;
class ChunkManager;

class EntityBehaviour
{
public:
    EntityBehaviour() = default;
    virtual ~EntityBehaviour() = default;

    virtual void update(Entity& entity, ChunkManager& chunkManager, Game& game, float dt) = 0;

    inline virtual void onHit(Entity& entity, Game& game, const LocationState& locationState, pl::Vector2f hitSource) {}
};