#pragma once

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/array.hpp>
#include <extlib/cereal/types/string.hpp>

#include "Network/IPacketData.hpp"
#include "Network/IPacketTimeDependent.hpp"

#include <Vector.hpp>

#include "Core/Tween.hpp"

#include "Data/typedefs.hpp"
#include "World/ChunkViewRange.hpp"

struct PacketDataPlayerCharacterInfo : public IPacketData, public IPacketTimeDependent
{
    float pingTime;

    pl::Vector2f position;
    pl::Vector2f direction;
    float speed;

    int animationFrame;
    float animationFrameTick;
    bool flipped;
    float yScaleMult;

    bool onWater;

    bool inRocket;
    
    ToolType toolType;
    // float toolRotation;
    bool fishingRodCasted;
    bool fishBitingLine;
    pl::Vector2<int> fishingRodBobWorldTile;

    bool usingTool;
    TweenID toolRotTweenID;
    TweenData<float> toolTweenData;

    std::array<ArmourType, 3> armour;

    ChunkViewRange chunkViewRange;
    uint64_t userID;

    inline virtual void applyPingCorrection(float pingTimeSecs) override
    {
        pingTime = pingTimeSecs;
    }

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(position.x, position.y, direction.x, direction.y, speed, animationFrame, flipped, yScaleMult, onWater, inRocket, toolType,
            fishingRodCasted, fishBitingLine, fishingRodBobWorldTile.x, fishingRodBobWorldTile.y, usingTool, toolRotTweenID, toolTweenData, armour,
            chunkViewRange, userID);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const
    {
        return PacketType::PlayerCharacterInfo;
    }
};