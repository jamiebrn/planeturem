#pragma once

#include <cstdint>
#include <optional>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/optional.hpp>

#include "Data/typedefs.hpp"
#include "Object/ObjectReference.hpp"

struct BuildableObjectPOD
{
    ObjectType objectType;
    uint16_t chestID;

    std::optional<ObjectReference> objectReference;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(objectType, chestID, objectReference);
    }
};