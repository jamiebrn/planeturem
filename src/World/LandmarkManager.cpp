#include "World/LandmarkManager.hpp"
#include "Player/Player.hpp"
#include "World/ChunkManager.hpp"
#include "Network/NetworkHandler.hpp"

void LandmarkManager::addLandmark(ObjectReference objectReference)
{
    landmarks.insert(objectReference);
}

void LandmarkManager::removeLandmark(ObjectReference objectReference)
{
    landmarks.erase(objectReference);
}

std::vector<LandmarkSummaryData> LandmarkManager::getLandmarkSummaryDatas(const Camera& camera, ChunkManager& chunkManager, NetworkHandler& networkHandler)
{
    std::vector<LandmarkSummaryData> landmarkSummaryDatas;

    for (auto iter = landmarks.begin(); iter != landmarks.end();)
    {
        Chunk* chunkPtr = chunkManager.getChunk(iter->chunk);

        if (!chunkPtr)
        {
            // Request chunk from host if is client
            if (networkHandler.isClient())
            {
                std::vector<ChunkPosition> chunksToRequest = {iter->chunk};
                networkHandler.requestChunksFromHost(chunkManager.getPlanetType(), chunksToRequest, true);
                iter++;
                continue;
            }

            // Remove landmark if not client (chunk does not exist)
            iter = landmarks.erase(iter);
            continue;
        }

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

int LandmarkManager::getLandmarkCount() const
{
    return landmarks.size();
}

void LandmarkManager::clear()
{
    landmarks.clear();
}