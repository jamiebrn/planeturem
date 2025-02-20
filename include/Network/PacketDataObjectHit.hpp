#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Object/ObjectReference.hpp"

struct PacketDataObjectHit : public IPacketData
{
    ObjectReference objectHit;
    int damage;
    uint64_t userId;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(objectHit, damage, userId);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::ObjectHit;
    }
};