#pragma once

#include "PacketDataBossSpawnRequest.hpp"

struct PacketDataBossSpawnCheckReply : public PacketDataBossSpawnRequest
{
    inline virtual PacketType getType() const override
    {
        return PacketType::BossSpawnCheckReply;
    }
};