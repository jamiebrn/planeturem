#pragma once

#include <unordered_set>
#include <vector>

#include <Graphics/Color.hpp>
#include <Vector.hpp>

#include "GameConstants.hpp"
#include "Object/ObjectReference.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/LandmarkObject.hpp"

class Player;
class ChunkManager;

struct LandmarkSummaryData
{
    pl::Vector2f worldPos;
    pl::Color colourA, colourB;
};

class LandmarkManager
{
public:
    LandmarkManager() = default;

    void addLandmark(ObjectReference objectReference);

    void removeLandmark(ObjectReference objectReference);

    // Gets world position for each landmark, using shortest distance from player
    // E.g. if player is at top of world, and landmark is at bottom of world, landmark position will be
    // given as a negative value from top of world
    std::vector<LandmarkSummaryData> getLandmarkSummaryDatas(const Player& player, ChunkManager& chunkManager);

    void clear();

private:
    std::unordered_set<ObjectReference> landmarks;
};