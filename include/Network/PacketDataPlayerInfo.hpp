#pragma once

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/array.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"

struct PacketDataPlayerInfo : public IPacketData
{
    float positionX;
    float positionY;

    int animationFrame;
    bool flipped;
    float yScaleMult;
    
    ToolType toolType;
    float toolRotation;

    std::array<ArmourType, 3> armour;

    uint64_t userID;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(positionX, positionY, animationFrame, flipped, yScaleMult, toolType, toolRotation, armour, userID);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const
    {
        return PacketType::PlayerInfo;
    }
};