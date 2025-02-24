#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "Object/ObjectReference.hpp"

struct PacketDataChestClosed : public IPacketData
{
    ObjectReference chestObject;
    uint64_t userID;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(chestObject, userID);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::ChestClosed;
    }
};