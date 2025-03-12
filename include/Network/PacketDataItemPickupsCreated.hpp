#pragma once

#include <vector>
#include <utility>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/utility.hpp>

#include "Network/IPacketData.hpp"

#include "Player/ItemPickup.hpp"
#include "Player/LocationState.hpp"

struct PacketDataItemPickupsCreated : public IPacketData
{
    LocationState locationState;
    std::vector<std::pair<ItemPickupReference, ItemPickup>> createdPickups;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(locationState, createdPickups);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::ItemPickupsCreated;
    }
};