#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"

struct PacketDataBossSpawnRequest : public IPacketData
{
    uint8_t planetType;
    ItemType bossSpawnItem;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(planetType, bossSpawnItem);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const override
    {
        return PacketType::BossSpawnRequest;
    }
};