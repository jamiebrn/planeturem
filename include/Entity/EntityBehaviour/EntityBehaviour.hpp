#pragma once

// #include <SFML/Graphics.hpp>

#include <Vector.hpp>
#include <Rect.hpp>

class Game;
class Entity;
class ChunkManager;

class EntityBehaviour
{
public:
    EntityBehaviour() = default;

    virtual void update(Entity& entity, ChunkManager& chunkManager, Game& game, float dt) = 0;

    inline virtual void onHit(Entity& entity, Game& game, pl::Vector2f hitSource) {}
};