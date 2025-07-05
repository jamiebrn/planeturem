#pragma once

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/string.hpp>

#include "Network/IPacketData.hpp"

struct PacketDataJoinQuery : public IPacketData
{
    bool requiresNameInput;

    std::string gameDataHash;
    
    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(requiresNameInput, gameDataHash);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const
    {
        return PacketType::JoinQuery;
    }
};