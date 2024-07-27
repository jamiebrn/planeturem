#include "Core/Camera.hpp"

// Initialise member variables, as is static class
sf::Vector2f Camera::offset = sf::Vector2f(0, 0);

// Update camera based on player position (or any position)
void Camera::update(sf::Vector2f playerPosition, float deltaTime)
{
    // Calculate position/offset camera should be in
    sf::Vector2f destinationOffset;
    destinationOffset.x = playerPosition.x - ResolutionHandler::getResolution().x / 2.0f;
    destinationOffset.y = playerPosition.y - ResolutionHandler::getResolution().y / 2.0f;

    // Interpolate towards desired position
    offset.x = Helper::lerp(offset.x, destinationOffset.x, MOVE_LERP_WEIGHT * deltaTime);
    offset.y = Helper::lerp(offset.y, destinationOffset.y, MOVE_LERP_WEIGHT * deltaTime);
}

// Get draw offset of camera
sf::Vector2f Camera::getDrawOffset()
{
    // Draw offset is negative camera position/offset
    sf::Vector2f drawOffset = -offset;
    
    return drawOffset;
}

sf::Vector2f Camera::getIntegerDrawOffset()
{
    sf::Vector2f drawOffset;
    drawOffset.x = static_cast<int>(-offset.x);
    drawOffset.y = static_cast<int>(-offset.y);
    
    return drawOffset;
}

// Set offset of camera
void Camera::setOffset(sf::Vector2f newOffset)
{
    offset = newOffset;
}

// Returns whether a specific world position with dimensions is in the camera view
bool Camera::isInView(sf::Vector2f position, sf::Vector2f size)
{
    // Calculate camera view world bounds (with object size)
    float minX = offset.x - size.x;
    float minY = offset.y - size.y;
    float maxX = offset.x + 1280 + size.x;
    float maxY = offset.y + 720 + size.y;

    // Return whether position is within bounds
    return (position.x >= minX && position.y >= minY && position.x <= maxX && position.y <= maxY);
}