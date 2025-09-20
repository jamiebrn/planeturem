#pragma once

#include <vector>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/utility.hpp>

#include <Vector.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "World/ChunkPosition.hpp"
#include "Player/LocationState.hpp"

struct ItemPickupRequest
{
    ChunkPosition chunk;
    pl::Vector2f positionRelative;
    ItemType itemType;
    uint16_t count;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(chunk, positionRelative.x, positionRelative.y, itemType, count);
    }
};

// Requests host to create item pickups
struct PacketDataItemPickupsCreateRequest : public IPacketData
{
    LocationState locationState;
    std::vector<ItemPickupRequest> pickupRequests;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(locationState, pickupRequests);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const override
    {
        return PacketType::ItemPickupsCreateRequest;
    }
};