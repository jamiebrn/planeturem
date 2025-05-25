#pragma once

#include <unordered_set>
#include <vector>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/unordered_set.hpp>

#include <Graphics/Color.hpp>
#include <Vector.hpp>

#include "GameConstants.hpp"
#include "Object/ObjectReference.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/LandmarkObject.hpp"
#include "Core/Camera.hpp"

class Player;
class ChunkManager;
class NetworkHandler;

struct LandmarkSummaryData
{
    pl::Vector2f screenPos;
    pl::Color colorA, colorB;
};

class LandmarkManager
{
public:
    LandmarkManager() = default;

    void addLandmark(ObjectReference objectReference);

    void removeLandmark(ObjectReference objectReference);

    std::vector<LandmarkSummaryData> getLandmarkSummaryDatas(const Camera& camera, ChunkManager& chunkManager, NetworkHandler& networkHandler);

    int getLandmarkCount() const;

    void clear();

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(landmarks);
    }

private:
    std::unordered_set<ObjectReference> landmarks;
};