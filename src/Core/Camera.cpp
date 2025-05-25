#include "Core/Camera.hpp"

bool Camera::screenShakeEnabled = true;

// Update camera based on player position (or any position)
void Camera::update(pl::Vector2f playerPosition, pl::Vector2f mouseScreenPos, float deltaTime)
{
    float scale = ResolutionHandler::getScale();

    pl::Vector2f resolution = static_cast<pl::Vector2f>(ResolutionHandler::getResolution());

    // Calculate position/offset camera should be in
    pl::Vector2f destinationOffset;
    destinationOffset.x = playerPosition.x - (resolution.x / scale) / 2.0f;
    destinationOffset.y = playerPosition.y - (resolution.y / scale) / 2.0f;

    // Add mouse delta
    destinationOffset.x += std::round((std::clamp(mouseScreenPos.x, 0.0f, resolution.x) - resolution.x / 2) / MOUSE_DELTA_DAMPEN / scale);
    destinationOffset.y += std::round((std::clamp(mouseScreenPos.y, 0.0f, resolution.y) - resolution.y / 2) / MOUSE_DELTA_DAMPEN / scale);

    // Apply screen shake if required
    screenShakeTime = std::max(screenShakeTime - deltaTime, 0.0f);
    if (screenShakeTime > 0.0f && screenShakeEnabled)
    {
        destinationOffset.x += Helper::randInt(-screenShakeTime * 150.0f / scale, screenShakeTime * 150.0f / scale);
        destinationOffset.y += Helper::randInt(-screenShakeTime * 150.0f / scale, screenShakeTime * 150.0f / scale);
    }

    // Interpolate towards desired position
    offset.x = Helper::lerp(offset.x, destinationOffset.x, MOVE_LERP_WEIGHT * deltaTime);
    offset.y = Helper::lerp(offset.y, destinationOffset.y, MOVE_LERP_WEIGHT * deltaTime);
}

void Camera::instantUpdate(pl::Vector2f playerPosition)
{
    float scale = ResolutionHandler::getScale();

    // Calculate position/offset camera should be in
    offset.x = playerPosition.x - (ResolutionHandler::getResolution().x / scale) / 2.0f;
    offset.y = playerPosition.y - (ResolutionHandler::getResolution().y / scale) / 2.0f;
}

// Get draw offset of camera
pl::Vector2f Camera::getDrawOffset() const
{
    // Draw offset is negative camera position/offset
    pl::Vector2f drawOffset = offset * -1;
    
    return drawOffset;
}

pl::Vector2f Camera::getIntegerDrawOffset() const
{
    pl::Vector2f drawOffset;
    drawOffset.x = static_cast<int>(-offset.x);
    drawOffset.y = static_cast<int>(-offset.y);
    
    return drawOffset;
}

pl::Vector2f Camera::translateWorldPos(pl::Vector2f position, pl::Vector2f origin, int worldSize)
{
    int worldPixelSize = worldSize * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    float halfWorldPixelSize = worldPixelSize / 2.0f;

    if (std::abs(origin.x - position.x) >= halfWorldPixelSize)
    {
        if (origin.x >= halfWorldPixelSize)
        {
            if (position.x < halfWorldPixelSize)
            {
                position.x += worldPixelSize;
            }
        }
        else
        {
            if (position.x >= halfWorldPixelSize)
            {
                position.x -= worldPixelSize;
            }
        }
    }

    if (std::abs(origin.y - position.y) >= halfWorldPixelSize)
    {
        if (origin.y >= halfWorldPixelSize)
        {
            if (position.y < halfWorldPixelSize)
            {
                position.y += worldPixelSize;
            }
        }
        else
        {
            if (position.y >= halfWorldPixelSize)
            {
                position.y -= worldPixelSize;
            }
        }
    }

    return position;
}

pl::Vector2f Camera::worldToScreenTransform(pl::Vector2f worldPos, int worldSize) const
{
    // Translate world pos if required (wraparound)
    if (worldSize > 0)
    {
        worldPos = translateWorldPos(worldPos, offset, worldSize);
    }

    float scale = ResolutionHandler::getScale();

    pl::Vector2f screenCentre = static_cast<pl::Vector2f>(ResolutionHandler::getResolution()) / 2.0f;
    screenCentre.x = std::round(screenCentre.x);
    screenCentre.y = std::round(screenCentre.y);
    pl::Vector2f screenCentreWorld = screenCentre / scale;

    pl::Vector2f screenPos;
    screenPos.x = static_cast<int>(std::round((worldPos.x - std::round(offset.x * scale) / scale - screenCentreWorld.x) * scale + screenCentre.x));
    screenPos.y = static_cast<int>(std::round((worldPos.y - std::round(offset.y * scale) / scale - screenCentreWorld.y) * scale + screenCentre.y));

    return screenPos;
}

pl::Vector2f Camera::screenToWorldTransform(pl::Vector2f screenPos, int worldSize) const
{
    float scale = ResolutionHandler::getScale();

    pl::Vector2f screenCentre = static_cast<pl::Vector2f>(ResolutionHandler::getResolution()) / 2.0f;
    pl::Vector2f screenCentreWorld = screenCentre / scale;
    
    pl::Vector2f worldPos;
    if (worldSize > 0)
    {
        int worldPixelSize = worldSize * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;

        worldPos.x = fmod(fmod((screenPos.x - screenCentre.x) / scale + screenCentreWorld.x + offset.x, worldPixelSize) + worldPixelSize, worldPixelSize);
        worldPos.y = fmod(fmod((screenPos.y - screenCentre.y) / scale + screenCentreWorld.y + offset.y, worldPixelSize) + worldPixelSize, worldPixelSize);
    }
    else
    {
        worldPos.x = (screenPos.x - screenCentre.x) / scale + screenCentreWorld.x + offset.x;
        worldPos.y = (screenPos.y - screenCentre.y) / scale + screenCentreWorld.y + offset.y;
    }

    return worldPos;
}

void Camera::handleScaleChange(float beforeScale, float afterScale, pl::Vector2f playerPosition)
{
    pl::Vector2f adjustedCamPos;
    adjustedCamPos.x = playerPosition.x - ((playerPosition.x - offset.x) * beforeScale) / afterScale;
    adjustedCamPos.y = playerPosition.y - ((playerPosition.y - offset.y) * beforeScale) / afterScale;
    Camera::setOffset(adjustedCamPos);
}

void Camera::handleWorldWrap(pl::Vector2f wrapPositionDelta)
{
    offset += wrapPositionDelta;
}

ChunkViewRange Camera::getChunkViewRange() const
{
    return getChunkViewRangeWithBorder(CHUNK_VIEW_LOAD_BORDER);
}

ChunkViewRange Camera::getChunkViewDrawRange() const
{
    return getChunkViewRangeWithBorder(0);
}

ChunkViewRange Camera::getChunkViewRangeWithBorder(int border) const
{
    // Use 0 for world size to prevent wrapping
    pl::Vector2f screenTopLeft = screenToWorldTransform({0, 0}, 0);
    pl::Vector2f screenBottomRight = screenToWorldTransform(static_cast<pl::Vector2f>(ResolutionHandler::getResolution()), 0);

    ChunkViewRange chunkViewRange;

    chunkViewRange.topLeft.x = std::floor(screenTopLeft.x / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE)) - border;
    chunkViewRange.topLeft.y = std::floor(screenTopLeft.y / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE)) - border;
    chunkViewRange.bottomRight.x = std::ceil(screenBottomRight.x / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE)) + border;
    chunkViewRange.bottomRight.y = std::ceil(screenBottomRight.y / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE)) + border;

    return chunkViewRange;
}

// Set offset of camera
void Camera::setOffset(pl::Vector2f newOffset)
{
    offset = newOffset;
}

// Returns whether a specific world position with dimensions is in the camera view
bool Camera::isInView(pl::Vector2f position, int worldSize) const
{
    pl::Vector2f screenPos = worldToScreenTransform(position, worldSize);

    return (screenPos.x >= 0 && screenPos.x <= ResolutionHandler::getResolution().x &&
            screenPos.y >= 0 && screenPos.y <= ResolutionHandler::getResolution().y);
}

void Camera::setScreenShakeTime(float time)
{
    screenShakeTime = time;
}