#include "World/LandmarkManager.hpp"
#include "Player/Player.hpp"

void LandmarkManager::addLandmark(ObjectReference objectReference)
{
    landmarks.insert(objectReference);
}

void LandmarkManager::removeLandmark(ObjectReference objectReference)
{
    landmarks.erase(objectReference);
}

std::vector<sf::Vector2f> LandmarkManager::getLandmarkWorldPositions(const Player& player, int worldSize)
{
    int worldPixelSize = worldSize * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    float halfWorldPixelSize = worldPixelSize / 2.0f;

    std::vector<sf::Vector2f> landmarkWorldPositions;

    for (auto iter = landmarks.begin(); iter != landmarks.end(); iter++)
    {
        sf::Vector2f landmarkWorldPos;
        landmarkWorldPos.x = (iter->chunk.x * CHUNK_TILE_SIZE + iter->tile.x) * TILE_SIZE_PIXELS_UNSCALED;
        landmarkWorldPos.y = (iter->chunk.y * CHUNK_TILE_SIZE + iter->tile.y) * TILE_SIZE_PIXELS_UNSCALED;

        if (player.getPosition().x >= halfWorldPixelSize)
        {
            if (landmarkWorldPos.x < halfWorldPixelSize)
            {
                landmarkWorldPos.x += worldPixelSize;
            }
        }
        else
        {
            if (landmarkWorldPos.x >= halfWorldPixelSize)
            {
                landmarkWorldPos.x -= worldPixelSize;
            }
        }

        if (player.getPosition().y >= halfWorldPixelSize)
        {
            if (landmarkWorldPos.y < halfWorldPixelSize)
            {
                landmarkWorldPos.y += worldPixelSize;
            }
        }
        else
        {
            if (landmarkWorldPos.y >= halfWorldPixelSize)
            {
                landmarkWorldPos.y -= worldPixelSize;
            }
        }

        landmarkWorldPositions.push_back(landmarkWorldPos);
    }

    return landmarkWorldPositions;
}