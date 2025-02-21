#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "Object/ObjectReference.hpp"

struct PacketDataObjectDestroyed : public IPacketData
{
    ObjectReference objectReference;
    uint64_t userId;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(objectReference, userId);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::ObjectDestroyed;
    }
};