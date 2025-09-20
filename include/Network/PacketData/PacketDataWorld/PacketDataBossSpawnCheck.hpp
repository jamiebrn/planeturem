#pragma once

#include "PacketDataBossSpawnRequest.hpp"

struct PacketDataBossSpawnCheck : public PacketDataBossSpawnRequest
{
    inline virtual PacketType getType() const override
    {
        return PacketType::BossSpawnCheck;
    }
};