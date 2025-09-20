#pragma once

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/array.hpp>

#include "Network/IPacketData.hpp"

#include "Player/PlayerData.hpp"

struct PacketDataPlayerData : public IPacketData
{
    uint64_t userID;

    PlayerData playerData;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(userID, playerData);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const override
    {
        return PacketType::PlayerData;
    }
};