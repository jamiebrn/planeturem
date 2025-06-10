#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Player/LocationState.hpp"
#include "Object/ObjectReference.hpp"

struct PacketDataRocketEnterReply : public IPacketData
{
    LocationState locationState;
    ObjectReference rocketObjectReference;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(locationState, rocketObjectReference);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::RocketEnterReply;
    }
};