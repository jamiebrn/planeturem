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

    inline virtual std::vector<char> serialise() const
    {
        std::stringstream stream;
        {
            cereal::BinaryOutputArchive archive(stream);
            archive(seed, gameTime, time, day, planetName);
        }

        std::string streamStr = stream.str();
        return std::vector<char>(streamStr.begin(), streamStr.end());
    }
    
    inline virtual void deserialise(const std::vector<char>& data)
    {
        std::stringstream stream(std::string(data.begin(), data.end()));
        cereal::BinaryInputArchive archive(stream);
        archive(seed, gameTime, time, day, planetName);
    }

    inline virtual PacketType getType() const
    {
        return PacketType::JoinInfo;
    }
};