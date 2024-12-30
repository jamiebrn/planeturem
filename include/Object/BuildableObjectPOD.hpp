#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/optional.hpp>
#include "Data/Serialise/ColorSerialise.hpp"

#include <SFML/Graphics/Color.hpp>

#include "Data/typedefs.hpp"
#include "Object/ObjectReference.hpp"

struct BuildableObjectPOD
{
    ObjectType objectType;
    uint16_t chestID = 0xFFFF;
    int plantDayPlanted = 0;

    std::optional<ObjectReference> objectReference;

    std::optional<sf::Color> landmarkColourA, landmarkColourB;

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        if (version == 1)
        {
            ar(objectType, chestID, objectReference, plantDayPlanted);
        }
        else if (version == 2)
        {
            sf::Color loadLandmarkColourA, loadLandmarkColourB;
            ar(objectType, chestID, objectReference, plantDayPlanted,
                loadLandmarkColourA.r, loadLandmarkColourA.g, loadLandmarkColourA.b,
                loadLandmarkColourB.r, loadLandmarkColourB.g, loadLandmarkColourB.b);
            
            landmarkColourA = loadLandmarkColourA;
            landmarkColourB = loadLandmarkColourB;
        }
        else if (version == 3)
        {
            ar(objectType, chestID, objectReference, plantDayPlanted,
                landmarkColourA, landmarkColourB);
        }
    }

    void mapVersions(const std::unordered_map<ObjectType, ObjectType>& objectVersionMap)
    {
        if (objectType < 0 || objectReference.has_value())
        {
            return;
        }

        objectType = objectVersionMap.at(objectType);
    }
};

CEREAL_CLASS_VERSION(BuildableObjectPOD, 3);