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
    float yScaleMult;
    
    // Bit packed values
    bool flipped;
    bool onWater;
    bool inRocket;
    bool fishingRodCasted;
    bool fishBitingLine;
    bool usingTool;
    
    pl::Vector2<int> fishingRodBobWorldTile;
    
    ToolType toolType;
    float toolRotation;
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
    void save(Archive& ar) const
    {
        uint8_t bitPacked = 0;
        std::vector<bool> bitPackValues = {flipped, onWater, inRocket, fishingRodCasted, fishBitingLine, usingTool};
        for (int i = 0; i < bitPackValues.size(); i++)
        {
            bitPacked |= ((bitPackValues[i] & 0b1) << i);
        }

        ar(position.x, position.y, direction.x, direction.y, speed, animationFrame, yScaleMult, toolType, toolRotation,
            fishingRodBobWorldTile.x, fishingRodBobWorldTile.y, toolRotTweenID, toolTweenData, armour, chunkViewRange, userID, bitPacked);
    }

    template <class Archive>
    void load(Archive& ar)
    {
        uint8_t bitPacked = 0;
        ar(position.x, position.y, direction.x, direction.y, speed, animationFrame, yScaleMult, toolType, toolRotation,
            fishingRodBobWorldTile.x, fishingRodBobWorldTile.y, toolRotTweenID, toolTweenData, armour, chunkViewRange, userID, bitPacked);
        
        std::vector<bool*> bitPackValues = {&flipped, &onWater, &inRocket, &fishingRodCasted, &fishBitingLine, &usingTool};
        for (int i = 0; i < bitPackValues.size(); i++)
        {
            *bitPackValues[i] = ((bitPacked >> i) & 0b1);
        }
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const
    {
        return PacketType::PlayerCharacterInfo;
    }
};