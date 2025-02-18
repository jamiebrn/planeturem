#pragma once

#include <extlib/cereal/archives/binary.hpp>
#include <cereal/types/polymorphic.hpp>

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

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(positionX, positionY, animationFrame, flipped, yScaleMult, toolType, toolRotation);
    }

    inline virtual PacketType getType() const
    {
        return PacketType::PlayerInfo;
    }
};

CEREAL_REGISTER_TYPE(PacketDataPlayerInfo);
CEREAL_REGISTER_POLYMORPHIC_RELATION(IPacketData, PacketDataPlayerInfo);