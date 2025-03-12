#pragma once

#include <vector>
#include <utility>

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Player/ItemPickup.hpp"
#include "Player/LocationState.hpp"

struct PacketDataItemPickupDeleted : public IPacketData
{
    LocationState locationState;
    ItemPickupReference pickupDeleted;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(locationState, pickupDeleted);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::ItemPickupDeleted;
    }
};