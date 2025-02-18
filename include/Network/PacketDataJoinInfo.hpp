#pragma once

#include <string>
#include <vector>

#include <extlib/cereal/types/string.hpp>
#include <extlib/cereal/types/vector.hpp>

#include "Network/IPacketData.hpp"

struct PacketDataJoinInfo : public IPacketData
{
    int seed;
    float gameTime;
    float time;
    int day;
    std::string planetName;

    std::vector<uint64_t> currentPlayers;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(seed, gameTime, time, day, planetName, currentPlayers);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::JoinInfo;
    }
};