#pragma once

#include <vector>
#include <array>
#include <optional>
#include <unordered_map>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/array.hpp>
#include <extlib/cereal/types/optional.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/string.hpp>

#include "Network/IPacketData.hpp"
#include "Network/IPacketTimeDependent.hpp"

#include "Network/CompactFloat.hpp"

#include "Data/typedefs.hpp"
#include "World/ChunkPosition.hpp"

struct PacketDataEntities : public IPacketData, public IPacketTimeDependent
{
    struct EntityPacketData
    {
        uint8_t entityType;
        CompactFloat<uint16_t> chunkRelativePositionX;
        CompactFloat<uint16_t> chunkRelativePositionY;
        CompactFloat<int16_t> velocityX;
        CompactFloat<int16_t> velocityY;

        ChunkPosition chunkPosition;
        
        uint8_t health;
        CompactFloat<uint8_t> flashAmount;
        uint8_t idleAnimFrame;
        uint8_t walkAnimFrame;

        template <class Archive>
        void serialize(Archive& ar, const std::uint32_t version)
        {
            ar(entityType, chunkRelativePositionX, chunkRelativePositionY, velocityX, velocityY,
                chunkPosition, health, flashAmount, idleAnimFrame, walkAnimFrame);
        }
    };

    float pingTime;

    uint8_t planetType;
    std::vector<EntityPacketData> entities;

    inline virtual void applyPingCorrection(float pingTimeSecs) override
    {
        pingTime = pingTimeSecs;
    }

    template <class Archive>
    void save(Archive& ar) const
    {
        bool hasEntities = (entities.size() > 0);
        ar(hasEntities, planetType);

        if (hasEntities)
        {
            ar(entities);
        }
    }

    template <class Archive>
    void load(Archive& ar)
    {
        bool hasEntities;
        ar(hasEntities, planetType);

        if (hasEntities)
        {
            ar(entities);
        }
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const override
    {
        return PacketType::Entities;
    }
};