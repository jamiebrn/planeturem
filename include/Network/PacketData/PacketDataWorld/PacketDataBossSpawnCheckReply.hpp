#pragma once

#include "PacketDataBossSpawnRequest.hpp"

struct PacketDataBossSpawnCheckReply : public PacketDataBossSpawnRequest
{
    inline virtual PacketType getType() const
    {
        return PacketType::BossSpawnCheckReply;
    }
};