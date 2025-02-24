#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Object/ObjectReference.hpp"
#include "Player/InventoryData.hpp"

struct PacketDataChestOpened : public IPacketData
{
    ObjectReference chestObject;
    uint64_t userID;

    // Only sent from host
    uint16_t chestID;
    InventoryData chestData;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(chestObject, userID, chestID, chestData);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::ChestOpened;
    }
};