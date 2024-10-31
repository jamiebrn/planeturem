#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/optional.hpp>

#include "Data/typedefs.hpp"
#include "Object/ObjectReference.hpp"

struct BuildableObjectPOD
{
    ObjectType objectType;
    uint16_t chestID = 0xFFFF;
    int plantDayPlanted = 0;

    std::optional<ObjectReference> objectReference;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(objectType, chestID, objectReference, plantDayPlanted);
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