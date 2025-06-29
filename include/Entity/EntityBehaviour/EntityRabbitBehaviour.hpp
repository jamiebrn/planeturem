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

    virtual void onHit(Entity& entity, Game& game, pl::Vector2f hitSource) override;

private:
    static constexpr float VELOCITY_MULT_LERP_WEIGHT = 1.9f;
    float velocityMult;

    static constexpr float MAX_IDLE_WAIT_TIME = 0.5f;
    float idleWaitTime;

    static constexpr float MIN_TARGET_RANGE = 32.0f;
    static constexpr float MAX_TARGET_RANGE = 128.0f;
    static constexpr float TARGET_RANGE_REACH_THRESHOLD = 4.0f;
    pl::Vector2f targetPosition;

};