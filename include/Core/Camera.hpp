#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

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
    void update(sf::Vector2f playerPosition, sf::Vector2f mouseScreenPos, float deltaTime);

    // Instantly set position to centre on player
    void instantUpdate(sf::Vector2f playerPosition);

    // Get draw offset of camera
    sf::Vector2f getDrawOffset() const;

    sf::Vector2f getIntegerDrawOffset() const;

    // Get scaled draw offset (applies zoom etc)
    sf::Vector2f worldToScreenTransform(sf::Vector2f worldPos) const;

    sf::Vector2f screenToWorldTransform(sf::Vector2f screenPos) const;

    void handleScaleChange(float beforeScale, float afterScale, sf::Vector2f playerPosition);

    void handleWorldWrap(sf::Vector2f positionDelta);

    ChunkViewRange getChunkViewRange() const;

    // Set offset of camera
    void setOffset(sf::Vector2f newOffset);

    bool isInView(sf::Vector2f position) const;

    void setScreenShakeTime(float time);

    inline static void setScreenShakeEnabled(float enabled) {screenShakeEnabled = enabled;}
    inline static bool getScreenShakeEnabled() {return screenShakeEnabled;}

private:
    static constexpr float MOVE_LERP_WEIGHT = 6;
    
    static constexpr float MOUSE_DELTA_DAMPEN = 15;

    sf::Vector2f offset;

    float screenShakeTime = 0.0f;

    static bool screenShakeEnabled;

};
