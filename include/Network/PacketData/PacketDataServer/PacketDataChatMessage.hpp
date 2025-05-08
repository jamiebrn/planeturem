#pragma once

#include <string>

#include <extlib/cereal/types/string.hpp>
#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

struct PacketDataChatMessage : public IPacketData
{
    uint64_t userId;

    std::string message;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(userId, message);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::ChatMessage;
    }
};