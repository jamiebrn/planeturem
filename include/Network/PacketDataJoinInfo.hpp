#pragma once

#include <string>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/string.hpp>

#include "Network/IPacketData.hpp"

struct PacketDataJoinInfo : public IPacketData
{
    int seed;
    float gameTime;
    float time;
    int day;
    std::string planetName;

    inline virtual std::string serialise() const
    {
        std::stringstream stream;
        {
            cereal::BinaryOutputArchive archive(stream);
            archive(seed, gameTime, time, day, planetName);
        }
        
        return stream.str();
    }
    
    inline virtual void deserialise(const std::string& data)
    {
        std::stringstream stream(data);
        cereal::BinaryInputArchive archive(stream);
        archive(seed, gameTime, time, day, planetName);
    }

    inline virtual PacketType getType() const
    {
        return PacketType::JoinInfo;
    }
};