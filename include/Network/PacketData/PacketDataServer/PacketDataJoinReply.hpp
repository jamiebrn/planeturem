#pragma once

#include <string>
#include <extlib/steam/steam_api.h>

#include <extlib/cereal/types/string.hpp>
#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

struct PacketDataJoinReply : public IPacketData
{
    std::string playerName;
    std::string pingLocation;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(playerName, pingLocation);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const
    {
        return PacketType::JoinReply;
    }
};