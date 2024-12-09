#include "World/LandmarkManager.hpp"
#include "Player/Player.hpp"
#include "World/ChunkManager.hpp"

void LandmarkManager::addLandmark(ObjectReference objectReference)
{
    landmarks.insert(objectReference);
}

void LandmarkManager::removeLandmark(ObjectReference objectReference)
{
    landmarks.erase(objectReference);
}

std::vector<LandmarkSummaryData> LandmarkManager::getLandmarkSummaryDatas(const Player& player, ChunkManager& chunkManager)
{
    int worldPixelSize = chunkManager.getWorldSize() * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    float halfWorldPixelSize = worldPixelSize / 2.0f;

    std::vector<LandmarkSummaryData> landmarkSummaryDatas;

    for (auto iter = landmarks.begin(); iter != landmarks.end();)
    {
        LandmarkSummaryData landmarkSummary;

        BuildableObject* objectPtr = chunkManager.getChunkObject(iter->chunk, iter->tile);

        LandmarkObject* landmarkObjectPtr = dynamic_cast<LandmarkObject*>(objectPtr);

        if (!landmarkObjectPtr)
        {
            iter = landmarks.erase(iter);
            continue;
        }

        landmarkSummary.worldPos = landmarkObjectPtr->getPosition();
        landmarkSummary.colourA = landmarkObjectPtr->getColourA();
        landmarkSummary.colourB = landmarkObjectPtr->getColourB();

        if (std::abs(player.getPosition().x - landmarkSummary.worldPos.x) >= halfWorldPixelSize)
        {
            if (player.getPosition().x >= halfWorldPixelSize)
            {
                if (landmarkSummary.worldPos.x < halfWorldPixelSize)
                {
                    landmarkSummary.worldPos.x += worldPixelSize;
                }
            }
            else
            {
                if (landmarkSummary.worldPos.x >= halfWorldPixelSize)
                {
                    landmarkSummary.worldPos.x -= worldPixelSize;
                }
            }
        }

        if (std::abs(player.getPosition().y - landmarkSummary.worldPos.y) >= halfWorldPixelSize)
        {
            if (player.getPosition().y >= halfWorldPixelSize)
            {
                if (landmarkSummary.worldPos.y < halfWorldPixelSize)
                {
                    landmarkSummary.worldPos.y += worldPixelSize;
                }
            }
            else
            {
                if (landmarkSummary.worldPos.y >= halfWorldPixelSize)
                {
                    landmarkSummary.worldPos.y -= worldPixelSize;
                }
            }
        }

        landmarkSummaryDatas.push_back(landmarkSummary);

        iter++;
    }

    return landmarkSummaryDatas;
}

void LandmarkManager::clear()
{
    landmarks.clear();
}