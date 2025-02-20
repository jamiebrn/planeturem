#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Object/ObjectReference.hpp"

struct PacketDataObjectHit : public IPacketData
{
    ObjectReference objectHit;
    int damage;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(objectHit, damage);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::ObjectHit;
    }
};