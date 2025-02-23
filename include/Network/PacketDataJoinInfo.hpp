#pragma once

#include <string>
#include <vector>

#include <extlib/cereal/types/string.hpp>
#include <extlib/cereal/types/vector.hpp>

#include <SFML/System/Vector2.hpp>

#include "Network/IPacketData.hpp"

struct PacketDataJoinInfo : public IPacketData
{
    int seed;
    float gameTime;
    float time;
    int day;
    std::string planetName;
    sf::Vector2f spawnPosition;

    std::vector<uint64_t> currentPlayers;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(seed, gameTime, time, day, planetName, spawnPosition.x, spawnPosition.y, currentPlayers);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::JoinInfo;
    }
};