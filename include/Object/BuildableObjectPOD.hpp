#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/optional.hpp>
#include "Data/Serialise/ColorSerialise.hpp"

// #include <SFML/Graphics/Color.hpp>

#include <Graphics/Color.hpp>

#include "Data/typedefs.hpp"
#include "Object/ObjectReference.hpp"

struct BuildableObjectPOD
{
    ObjectType objectType;
    uint16_t chestID = 0xFFFF;
    int plantDayPlanted = 0;

    std::optional<ObjectReference> objectReference;

    std::optional<pl::Color> landmarkColorA, landmarkColorB;

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        if (version == 1)
        {
            ar(objectType, chestID, objectReference, plantDayPlanted);
        }
        else if (version == 2)
        {
            uint8_t loadLandmarkColorAR, loadLandmarkColorAG, loadLandmarkColorAB, loadLandmarkColouBR, loadLandmarkColouBG, loadLandmarkColouBB;
            ar(objectType, chestID, objectReference, plantDayPlanted,
                loadLandmarkColorAR, loadLandmarkColorAG, loadLandmarkColorAB,
                loadLandmarkColouBR, loadLandmarkColouBG, loadLandmarkColouBB);
            
            landmarkColorA = pl::Color(loadLandmarkColorAR, loadLandmarkColorAG, loadLandmarkColorAB);
            landmarkColorB = pl::Color(loadLandmarkColouBR, loadLandmarkColouBG, loadLandmarkColouBB);
        }
        else if (version == 3)
        {
            uint8_t loadLandmarkColorAR, loadLandmarkColorAG, loadLandmarkColorAB, loadLandmarkColouBR, loadLandmarkColouBG, loadLandmarkColouBB;
            ar(objectType, chestID, objectReference, plantDayPlanted,
                loadLandmarkColorAR, loadLandmarkColorAG, loadLandmarkColorAB, loadLandmarkColouBR, loadLandmarkColouBG, loadLandmarkColouBB);

            landmarkColorA = pl::Color(loadLandmarkColorAR, loadLandmarkColorAG, loadLandmarkColorAB);
            landmarkColorB = pl::Color(loadLandmarkColouBR, loadLandmarkColouBG, loadLandmarkColouBB);
        }
        if (version == 4)
        {
            ar(objectType, chestID, objectReference, plantDayPlanted, landmarkColorA, landmarkColorB);
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

CEREAL_CLASS_VERSION(BuildableObjectPOD, 4);