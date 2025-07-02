#pragma once

#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Helper.hpp"

#include "Entity/EntityBehaviour/EntityBehaviour.hpp"

class EntityWanderBehaviour : public EntityBehaviour
{
public:
    EntityWanderBehaviour(Entity& entity);

    void update(Entity& entity, ChunkManager& chunkManager, Game& game, float dt) override;

    virtual void onHit(Entity& entity, Game& game, const LocationState& locationState, pl::Vector2f hitSource) override;

private:
    static constexpr float VELOCITY_MULT_LERP_WEIGHT = 1.9f;
    float velocityMult;

};