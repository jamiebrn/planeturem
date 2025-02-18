#pragma once

#include <string>

#include <extlib/cereal/types/string.hpp>
#include <cereal/types/polymorphic.hpp>

#include "Network/IPacketData.hpp"

struct PacketDataJoinInfo : public IPacketData
{
    int seed;
    float gameTime;
    float time;
    int day;
    std::string planetName;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(seed, gameTime, time, day, planetName);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::JoinInfo;
    }
};