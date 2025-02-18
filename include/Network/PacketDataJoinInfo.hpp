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
    
    inline virtual PacketType getType() const
    {
        return PacketType::JoinInfo;
    }
};

CEREAL_REGISTER_TYPE(PacketDataJoinInfo);
CEREAL_REGISTER_POLYMORPHIC_RELATION(IPacketData, PacketDataJoinInfo);