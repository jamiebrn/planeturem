#pragma once

#include <string>
#include <extlib/steam/steam_api.h>

#include <extlib/cereal/types/string.hpp>
#include <extlib/cereal/archives/binary.hpp>

#include "Data/Serialise/ColorSerialise.hpp"

#include "Network/IPacketData.hpp"

struct PacketDataJoinReply : public IPacketData
{
    std::string playerName;
    pl::Color bodyColor;
    pl::Color skinColor;
    std::string pingLocation;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(playerName, bodyColor, skinColor, pingLocation);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const override
    {
        return PacketType::JoinReply;
    }
};