#pragma once

#include <algorithm>
#include <cmath>

#include "Vector.hpp"

#include "Core/ResolutionHandler.hpp"
#include "Core/Helper.hpp"

#include "World/ChunkViewRange.hpp"

#include "GameConstants.hpp"

class Camera
{

private:

public:
    Camera() = default;

    // Update camera based on player position (or any position)
    void update(pl::Vector2f playerPosition, pl::Vector2f mouseScreenPos, float deltaTime);

    // Instantly set position to centre on player
    void instantUpdate(pl::Vector2f playerPosition);

    // Get draw offset of camera
    pl::Vector2f getDrawOffset() const;

    pl::Vector2f getIntegerDrawOffset() const;

    static pl::Vector2f translateWorldPos(pl::Vector2f position, pl::Vector2f origin, int worldSize);

    // Use worldSize = 0 to disable planet wrapping
    pl::Vector2f worldToScreenTransform(pl::Vector2f worldPos, int worldSize) const;

    pl::Vector2f screenToWorldTransform(pl::Vector2f screenPos, int worldSize) const;

    void handleScaleChange(float beforeScale, float afterScale, pl::Vector2f playerPosition);

    void handleWorldWrap(pl::Vector2f wrapPositionDelta);

    ChunkViewRange getChunkViewRange() const;
    ChunkViewRange getChunkViewDrawRange() const;
    
    // Set offset of camera
    void setOffset(pl::Vector2f newOffset);
    
    bool isInView(pl::Vector2f position, int worldSize) const;
    
    void setScreenShakeTime(float time);
    
    inline static void setScreenShakeEnabled(float enabled) {screenShakeEnabled = enabled;}
    inline static bool getScreenShakeEnabled() {return screenShakeEnabled;}
    
private:
    ChunkViewRange getChunkViewRangeWithBorder(int border = 0) const;

    static constexpr float MOVE_LERP_WEIGHT = 6;
    
    static constexpr float MOUSE_DELTA_DAMPEN = 15;

    pl::Vector2f offset;

    float screenShakeTime = 0.0f;

    static bool screenShakeEnabled;

};
