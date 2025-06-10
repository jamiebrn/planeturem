#pragma once

#include <cstdint>
#include <optional>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/optional.hpp>

#include <Vector.hpp>

#include "Data/typedefs.hpp"

struct EntityPOD
{
    EntityType entityType;
    pl::Vector2f chunkRelativePosition;
    pl::Vector2f velocity;

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        ar(entityType, chunkRelativePosition.x, chunkRelativePosition.y, velocity.x, velocity.y);
    }
};

CEREAL_CLASS_VERSION(EntityPOD, 1);