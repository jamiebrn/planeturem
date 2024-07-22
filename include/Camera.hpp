#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>

// Include headers
#include "Helper.hpp"

// Declare camera class
class Camera
{

// Delete contructor, as is static class
private:
    Camera() = delete;

// Public class functions
public:
    // Update camera based on player position (or any position)
    static void update(sf::Vector2f playerPosition, float deltaTime);

    // Get draw offset of camera
    static sf::Vector2f getDrawOffset();

    // Set offset of camera
    static void setOffset(sf::Vector2f newOffset);

    // Returns whether a specific world position with dimensions is in the camera view
    static bool isInView(sf::Vector2f position, sf::Vector2f size);

// Private member variables
private:
    // Constant storing interpolation weight for camera movement
    static constexpr float MOVE_LERP_WEIGHT = 6;

    // Variable storing offset/position of camera
    static sf::Vector2f offset;

};
