#pragma once

#include "PacketDataBossSpawnRequest.hpp"

struct PacketDataBossSpawnCheck : public PacketDataBossSpawnRequest
{
    inline virtual PacketType getType() const
    {
        return PacketType::BossSpawnCheck;
    }
};