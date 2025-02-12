#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

// Include headers
#include "Core/ResolutionHandler.hpp"
#include "Core/Helper.hpp"

#include "GameConstants.hpp"

// Declare camera class
class Camera
{

private:

// Public class functions
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

    // Set offset of camera
    void setOffset(sf::Vector2f newOffset);

    bool isInView(sf::Vector2f position) const;

    void setScreenShakeTime(float time);

    inline static void setScreenShakeEnabled(float enabled) {screenShakeEnabled = enabled;}
    inline static bool getScreenShakeEnabled() {return screenShakeEnabled;}

// Private member variables
private:
    // Constant storing interpolation weight for camera movement
    static constexpr float MOVE_LERP_WEIGHT = 6;
    
    static constexpr float MOUSE_DELTA_DAMPEN = 15;

    // Variable storing offset/position of camera
    sf::Vector2f offset;

    float screenShakeTime = 0.0f;

    static bool screenShakeEnabled;

};
