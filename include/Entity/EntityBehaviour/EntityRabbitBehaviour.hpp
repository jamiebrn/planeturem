#pragma once

#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Helper.hpp"

#include "Entity/EntityBehaviour/EntityBehaviour.hpp"

class EntityRabbitBehaviour : public EntityBehaviour
{
public:
    EntityRabbitBehaviour(Entity& entity);

    void update(Entity& entity, ChunkManager& chunkManager, Game& game, float dt) override;

    virtual void onHit(Entity& entity, Game& game, const LocationState& locationState, pl::Vector2f hitSource) override;

private:
    static constexpr float VELOCITY_MULT_LERP_WEIGHT = 1.9f;
    float velocityMult;

    float walkSpeed = 0.0f;

    float minIdleWaitTime = 0.0f;
    float maxIdleWaitTime = 0.0f;
    float idleWaitTime;

    static constexpr float MIN_TARGET_RANGE = 20.0f;
    static constexpr float MAX_TARGET_RANGE = 64.0f;
    static constexpr float TARGET_RANGE_REACH_THRESHOLD = 4.0f;
    pl::Vector2f targetPosition;

};