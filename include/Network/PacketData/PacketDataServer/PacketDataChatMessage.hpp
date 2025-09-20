#pragma once

#include <string>
#include <optional>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/string.hpp>
#include <extlib/cereal/types/optional.hpp>

#include "Network/IPacketData.hpp"

struct PacketDataChatMessage : public IPacketData
{
    std::optional<uint64_t> userId;

    std::string message;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(userId, message);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const override
    {
        return PacketType::ChatMessage;
    }
};