#pragma once

#include <vector>
#include <utility>

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Player/ItemPickup.hpp"

struct PacketDataItemPickupDeleted : public IPacketData
{
    ItemPickupReference pickupDeleted;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(pickupDeleted);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::ItemPickupDeleted;
    }
};