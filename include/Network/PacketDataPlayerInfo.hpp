#pragma once

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/array.hpp>

#include "Network/IPacketData.hpp"

#include <SFML/System/Vector2.hpp>

#include "Data/typedefs.hpp"

struct PacketDataPlayerInfo : public IPacketData
{
    float positionX;
    float positionY;

    int animationFrame;
    bool flipped;
    float yScaleMult;

    bool onWater;
    
    ToolType toolType;
    float toolRotation;
    bool fishingRodCasted;
    sf::Vector2i fishingRodBobWorldTile;

    std::array<ArmourType, 3> armour;

    uint64_t userID;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(positionX, positionY, animationFrame, flipped, yScaleMult, onWater, toolType, toolRotation,
            fishingRodCasted, fishingRodBobWorldTile.x, fishingRodBobWorldTile.y, armour, userID);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const
    {
        return PacketType::PlayerInfo;
    }
};