#pragma once

#include <cstdint>
#include <optional>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/optional.hpp>

#include "Core/CollisionRect.hpp"

#include "Data/typedefs.hpp"

struct StructureObjectPOD
{
    StructureType structureType;

    sf::Vector2f relativePos;
    std::optional<CollisionRect> warpEntranceRectRelative = std::nullopt;

    uint32_t structureID;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(structureType, relativePos.x, relativePos.y, warpEntranceRectRelative, structureID);
    }
};