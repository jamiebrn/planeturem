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
    std::vector<LandmarkSummaryData> landmarkSummaryDatas;

    for (auto iter = landmarks.begin(); iter != landmarks.end();)
    {
        LandmarkSummaryData landmarkSummary;

        LandmarkObject* landmarkObjectPtr = chunkManager.getChunkObject<LandmarkObject>(iter->chunk, iter->tile);

        if (!landmarkObjectPtr)
        {
            iter = landmarks.erase(iter);
            continue;
        }

        landmarkSummary.worldPos = landmarkObjectPtr->getPosition();
        landmarkSummary.colourA = landmarkObjectPtr->getColourA();
        landmarkSummary.colourB = landmarkObjectPtr->getColourB();

        landmarkSummary.worldPos = chunkManager.translatePositionAroundWorld(landmarkSummary.worldPos, player.getPosition());

        landmarkSummaryDatas.push_back(landmarkSummary);

        iter++;
    }

    return landmarkSummaryDatas;
}

void LandmarkManager::clear()
{
    landmarks.clear();
}