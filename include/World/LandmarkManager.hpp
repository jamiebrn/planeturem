#pragma once

#include <unordered_set>
#include <vector>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/unordered_set.hpp>

#include <SFML/System/Vector2.hpp>

#include "GameConstants.hpp"
#include "Object/ObjectReference.hpp"

class Player;

class LandmarkManager
{
public:
    LandmarkManager() = default;

    void addLandmark(ObjectReference objectReference);

    void removeLandmark(ObjectReference objectReference);

    // Gets world position for each landmark, using shortest distance from player
    // E.g. if player is at top of world, and landmark is at bottom of world, landmark position will be
    // given as a negative value from top of world
    std::vector<sf::Vector2f> getLandmarkWorldPositions(const Player& player, int worldSize);

    void clear();

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        if (version == 1)
        {
            ar(landmarks);
        }
    }

private:
    std::unordered_set<ObjectReference> landmarks;
};

CEREAL_CLASS_VERSION(LandmarkManager, 1);