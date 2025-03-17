#pragma once

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/array.hpp>

#include "Network/IPacketData.hpp"

#include <SFML/System/Vector2.hpp>

#include "Data/typedefs.hpp"
#include "World/ChunkViewRange.hpp"

struct PacketDataPlayerCharacterInfo : public IPacketData
{
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed;

    int animationFrame;
    bool flipped;
    float yScaleMult;

    bool onWater;

    bool inRocket;
    
    ToolType toolType;
    float toolRotation;
    bool fishingRodCasted;
    bool fishBitingLine;
    sf::Vector2i fishingRodBobWorldTile;

    std::array<ArmourType, 3> armour;

    ChunkViewRange chunkViewRange;
    uint64_t userID;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(position.x, position.y, direction.x, direction.y, speed, animationFrame, flipped, yScaleMult, onWater, inRocket, toolType, toolRotation,
            fishingRodCasted, fishBitingLine, fishingRodBobWorldTile.x, fishingRodBobWorldTile.y, armour,
            chunkViewRange, userID);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const
    {
        return PacketType::PlayerCharacterInfo;
    }
};