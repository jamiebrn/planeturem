#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

struct PacketDataJoinQuery : public IPacketData
{
    bool requiresNameInput;
    
    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(requiresNameInput);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const
    {
        return PacketType::JoinQuery;
    }
};