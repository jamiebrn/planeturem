#pragma once

#include <string>
#include <vector>
#include <sstream>

#include <extlib/cereal/archives/binary.hpp>

#include "Network/PacketType.hpp"

struct IPacketData
{
    inline std::vector<char> serialise() const
    {
        std::stringstream stream;
        {
            cereal::BinaryOutputArchive archive(stream);
            archive(*this);
        }

        std::string streamStr = stream.str();
        return std::vector<char>(streamStr.begin(), streamStr.end());
    }
    
    inline void deserialise(const std::vector<char>& data)
    {
        std::stringstream stream(std::string(data.begin(), data.end()));
        cereal::BinaryInputArchive archive(stream);
        archive(*this);
    }

    template <class Archive>
    void serialize(Archive& ar)
    {
    }

    virtual PacketType getType() const = 0;
};