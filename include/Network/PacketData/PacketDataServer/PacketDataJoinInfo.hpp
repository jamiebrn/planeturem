#pragma once

#include <string>
#include <vector>

#include <extlib/cereal/types/unordered_map.hpp>
#include <extlib/cereal/archives/binary.hpp>

#include <SFML/System/Vector2.hpp>

#include "Network/IPacketData.hpp"

#include "Player/PlayerData.hpp"
#include "World/ChestDataPool.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"

struct PacketDataJoinInfo : public IPacketData
{
    int seed;
    float gameTime;
    float time;
    int day;
    // std::string planetName;
    // sf::Vector2f spawnPosition;

    PlayerData playerData;

    // ChestDataPool chestDataPool;

    std::unordered_map<uint64_t, PlayerData> currentPlayerDatas;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(seed, gameTime, time, day, playerData, currentPlayerDatas);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::JoinInfo;
    }
};