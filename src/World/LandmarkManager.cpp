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

std::vector<LandmarkSummaryData> LandmarkManager::getLandmarkSummaryDatas(const Camera& camera, ChunkManager& chunkManager)
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

        landmarkSummary.colorA = landmarkObjectPtr->getColorA();
        landmarkSummary.colorB = landmarkObjectPtr->getColorB();

        landmarkSummary.screenPos = camera.worldToScreenTransform(landmarkObjectPtr->getPosition(), chunkManager.getWorldSize());

        landmarkSummaryDatas.push_back(landmarkSummary);

        iter++;
    }

    return landmarkSummaryDatas;
}

void LandmarkManager::clear()
{
    landmarks.clear();
}