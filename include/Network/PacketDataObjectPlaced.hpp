#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "Object/ObjectReference.hpp"

struct PacketDataObjectPlaced : public IPacketData
{
    ObjectReference objectReference;
    ObjectType objectType;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(objectReference, objectType);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::ObjectPlaced;
    }
};