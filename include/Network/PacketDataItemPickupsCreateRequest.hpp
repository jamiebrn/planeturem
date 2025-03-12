#pragma once

#include <vector>
#include <utility>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/utility.hpp>

#include <SFML/System/Vector2.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "World/ChunkPosition.hpp"
#include "Player/LocationState.hpp"

struct ItemPickupRequest
{
    ChunkPosition chunk;
    sf::Vector2f positionRelative;
    ItemType itemType;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(chunk, positionRelative.x, positionRelative.y, itemType);
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
    
    inline virtual PacketType getType() const
    {
        return PacketType::ItemPickupsCreateRequest;
    }
};