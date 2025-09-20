#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "Object/ObjectReference.hpp"

struct PacketDataRoomTravelRequest : public IPacketData
{
    RoomType roomType;
    ObjectReference rocketUsedReference; // used when travelling from another planet, so rocket can be removed

    uint64_t userId;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(roomType, rocketUsedReference);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const override
    {
        return PacketType::RoomTravelRequest;
    }
};