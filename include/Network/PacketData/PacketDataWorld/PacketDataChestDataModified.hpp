#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Player/LocationState.hpp"
#include "Player/InventoryData.hpp"

struct PacketDataChestDataModified : public IPacketData
{
    LocationState locationState;
    uint16_t chestID;
    InventoryData chestData;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(locationState, chestID, chestData);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::ChestDataModified;
    }
};