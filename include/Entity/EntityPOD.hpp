#pragma once

#include <cstdint>
#include <optional>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/optional.hpp>
#include <SFML/System/Vector2.hpp>

#include "Data/typedefs.hpp"

struct EntityPOD
{
    EntityType entityType;
    sf::Vector2f chunkRelativePosition;
    sf::Vector2f velocity;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(entityType, chunkRelativePosition.x, chunkRelativePosition.y, velocity.x, velocity.y);
    }
};