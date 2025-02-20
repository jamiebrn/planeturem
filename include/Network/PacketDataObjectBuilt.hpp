#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "Object/ObjectReference.hpp"

struct PacketDataObjectBuilt : public IPacketData
{
    ObjectReference objectReference;
    ObjectType objectType;
    uint64_t userId;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(objectReference, objectType, userId);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::ObjectBuilt;
    }
};