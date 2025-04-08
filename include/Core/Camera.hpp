#pragma once

// #include <SFML/Graphics.hpp>
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

    // Get scaled draw offset (applies zoom etc)
    pl::Vector2f worldToScreenTransform(pl::Vector2f worldPos) const;

    pl::Vector2f screenToWorldTransform(pl::Vector2f screenPos) const;

    void handleScaleChange(float beforeScale, float afterScale, pl::Vector2f playerPosition);

    void handleWorldWrap(pl::Vector2f positionDelta);

    ChunkViewRange getChunkViewRange() const;

    // Set offset of camera
    void setOffset(pl::Vector2f newOffset);

    bool isInView(pl::Vector2f position) const;

    void setScreenShakeTime(float time);

    inline static void setScreenShakeEnabled(float enabled) {screenShakeEnabled = enabled;}
    inline static bool getScreenShakeEnabled() {return screenShakeEnabled;}

private:
    static constexpr float MOVE_LERP_WEIGHT = 6;
    
    static constexpr float MOUSE_DELTA_DAMPEN = 15;

    pl::Vector2f offset;

    float screenShakeTime = 0.0f;

    static bool screenShakeEnabled;

};
