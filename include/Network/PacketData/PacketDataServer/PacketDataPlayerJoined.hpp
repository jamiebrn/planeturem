#pragma once

#include <string>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/string.hpp>

#include "Network/IPacketData.hpp"

struct PacketDataPlayerJoined : public IPacketData
{
    uint64_t id;
    std::string name;
    std::string pingLocation;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(id, name, pingLocation);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const override
    {
        return PacketType::PlayerJoined;
    }
};