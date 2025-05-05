#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "Object/ObjectReference.hpp"

struct PacketDataRoomTravelReply : public IPacketData
{
    RoomType roomType;
    ObjectReference rocketObjectReference;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(roomType, rocketObjectReference);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::PlanetTravelReply;
    }
};