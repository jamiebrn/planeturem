#pragma once

#include <string>
#include <optional>

#include <extlib/cereal/types/unordered_map.hpp>
#include <extlib/cereal/types/optional.hpp>
#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "Player/PlayerData.hpp"
#include "World/ChestDataPool.hpp"
#include "World/WorldMap.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"

#include "Network/PacketData/PacketDataWorld/PacketDataLandmarks.hpp"

struct PacketDataJoinInfo : public IPacketData
{
    int seed;
    float gameTime;
    float time;
    int day;

    PlayerData playerData;

    std::unordered_map<uint64_t, PlayerData> currentPlayerDatas;

    std::optional<PacketDataLandmarks> landmarks;
    WorldMap worldMap;

    // If player previously saved in structure, send room type
    std::optional<RoomType> inStructureRoomType;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(seed, gameTime, time, day, playerData, currentPlayerDatas, landmarks, worldMap, inStructureRoomType);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const override
    {
        return PacketType::JoinInfo;
    }
};