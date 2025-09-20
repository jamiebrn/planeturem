#pragma once

#include <vector>
#include <utility>

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Player/ItemPickup.hpp"
#include "Player/LocationState.hpp"

struct PacketDataItemPickupCollected : public IPacketData
{
    LocationState locationState;
    ItemPickupReference pickup;
    uint16_t count;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(locationState, pickup, count);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const override
    {
        return PacketType::ItemPickupCollected;
    }
};