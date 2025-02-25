#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Player/InventoryData.hpp"

struct PacketDataChestDataModified : public IPacketData
{
    uint16_t chestID;
    InventoryData chestData;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(chestID, chestData);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::ChestDataModified;
    }
};