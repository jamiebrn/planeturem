#pragma once

#include <vector>
#include <array>
#include <optional>
#include <unordered_map>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/array.hpp>
#include <extlib/cereal/types/optional.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/unordered_map.hpp>

#include <SFML/System/Vector2.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "World/ChunkPosition.hpp"
#include "Entity/EntityPOD.hpp"

struct PacketDataEntities : public IPacketData
{
    struct EntityPacketData : public EntityPOD
    {
        int health;
        float flashAmount;
        int8_t idleAnimFrame;
        int8_t walkAnimFrame;

        template <class Archive>
        void serialize(Archive& ar, const std::uint32_t version)
        {
            ar(entityType, chunkRelativePosition.x, chunkRelativePosition.y, velocity.x, velocity.y,
                health, flashAmount, idleAnimFrame, walkAnimFrame);
        }
    };

    PlanetType planetType;
    std::unordered_map<ChunkPosition, std::vector<EntityPacketData>> entities;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(planetType, entities);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const
    {
        return PacketType::Entities;
    }
};