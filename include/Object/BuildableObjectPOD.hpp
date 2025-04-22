#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/optional.hpp>
#include "Data/Serialise/ColorSerialise.hpp"

#include <Graphics/Color.hpp>

#include "Data/typedefs.hpp"
#include "Object/ObjectReference.hpp"
#include "Data/ObjectDataLoader.hpp"

struct BuildableObjectPOD
{
    ObjectType objectType;

    uint8_t attributeMask = 0;
    
    uint16_t chestID = 0xFFFF;
    uint32_t plantDayPlanted = 0;
    
    std::optional<ObjectReference> objectReference;
    
    std::optional<pl::Color> landmarkColorA, landmarkColorB;
    
    template <class Archive>
    void save(Archive& ar, const std::uint32_t version) const
    {
        uint8_t attributeMask = 0;
        
        if (version == 5)
        {
            // Create attribute bitmask
            if (chestID != 0xFFFF) attributeMask |= 0x1;
            if (plantDayPlanted > 0) attributeMask |= 0x2;
            if (objectReference.has_value()) attributeMask |= 0x4;
            if (landmarkColorA.has_value()) attributeMask |= 0x8;
            if (landmarkColorB.has_value()) attributeMask |= 0x16;
            
            ar(objectType, attributeMask);
            
            // Serialise attributes
            if (attributeMask & 0x1) ar(chestID);
            if (attributeMask & 0x2) ar(plantDayPlanted);
            if (attributeMask & 0x4) ar(objectReference.value());
            if (attributeMask & 0x8) ar(landmarkColorA.value());
            if (attributeMask & 0x16) ar(landmarkColorB.value());
        }
    }
    
    template <class Archive>
    void load(Archive& ar, const std::uint32_t version)
    {
        if (version == 5)
        {
            ar(objectType, attributeMask);

            if (attributeMask & 0x1) ar(chestID);
            if (attributeMask & 0x2) ar(plantDayPlanted);
            if (attributeMask & 0x4)
            {
                ObjectReference objectReferenceSerialised;
                ar(objectReferenceSerialised);
                objectReference = objectReferenceSerialised;
            }
            if (attributeMask & 0x8)
            {
                pl::Color landmarkColorASerialised;
                ar(landmarkColorASerialised);
                landmarkColorA = landmarkColorASerialised;
            }
            if (attributeMask & 0x16)
            {
                pl::Color landmarkColorBSerialised;
                ar(landmarkColorBSerialised);
                landmarkColorB = landmarkColorBSerialised;
            }
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

CEREAL_CLASS_VERSION(BuildableObjectPOD, 5);