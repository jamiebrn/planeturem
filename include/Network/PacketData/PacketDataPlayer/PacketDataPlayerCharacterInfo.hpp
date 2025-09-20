#pragma once

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/array.hpp>
#include <extlib/cereal/types/string.hpp>

#include "Network/IPacketData.hpp"
#include "Network/IPacketTimeDependent.hpp"
#include "Network/CompactFloat.hpp"

#include <Vector.hpp>

#include "Core/Tween.hpp"

#include "Data/typedefs.hpp"
#include "World/ChunkViewRange.hpp"

struct PacketDataPlayerCharacterInfo : public IPacketData, public IPacketTimeDependent
{
    float pingTime;

    pl::Vector2f position;

    pl::Vector2f direction;

    int16_t health;

    uint8_t animationFrame;
    float animationFrameTick;
    float yScaleMult;
    
    // Bit packed values
    bool flipped;
    bool onWater;
    bool inRocket;
    bool fishingRodCasted;
    bool fishBitingLine;
    bool usingTool;
    
    pl::Vector2<uint16_t> fishingRodBobWorldTile;
    
    int8_t toolType;
    float toolRotation;

    std::array<int8_t, 3> armour;

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
        std::vector<const bool*> bitPackValues = getBitPackValues();
        for (int i = 0; i < bitPackValues.size(); i++)
        {
            bitPacked |= (((*bitPackValues[i]) & 0b1) << i);
        }

        CompactFloat<uint8_t> animationFrameTickCompact(animationFrameTick, 2);
        CompactFloat<uint8_t> yScaleMultCompact(yScaleMult, 2);

        CompactFloat<int16_t> toolRotationCompact(toolRotation, 2);
        
        CompactFloat<int8_t> directionXCompact(direction.x, 2);
        CompactFloat<int8_t> directionYCompact(direction.y, 2);

        // Compress position

        // Chunk inside
        uint8_t chunkPositionX = std::floor(position.x / (CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED));
        uint8_t chunkPositionY = std::floor(position.y / (CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED));

        // Position in current chunk (uses 22 bits)
        uint32_t positionDataLocal = 0;
        positionDataLocal |= static_cast<uint32_t>(fmod(position.x, CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED) * 10) & 0x7FF;
        positionDataLocal |= (static_cast<uint32_t>(fmod(position.y, CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED) * 10) & 0x7FF) << 11;

        // Use unused bits to store chunkPositionY
        positionDataLocal |= chunkPositionY << 22;

        ar(chunkPositionX, positionDataLocal, directionXCompact, directionYCompact, health, animationFrame, animationFrameTickCompact, yScaleMultCompact,
            toolType, toolRotationCompact, armour, chunkViewRange, userID, bitPacked);
        
        if (fishingRodCasted)
        {
            ar(fishingRodBobWorldTile.x, fishingRodBobWorldTile.y);
        }
    }

    template <class Archive>
    void load(Archive& ar)
    {
        uint8_t bitPacked = 0;
        CompactFloat<uint8_t> animationFrameTickCompact;
        CompactFloat<uint8_t> yScaleMultCompact;

        CompactFloat<int16_t> toolRotationCompact;

        CompactFloat<int8_t> directionXCompact;
        CompactFloat<int8_t> directionYCompact;

        uint8_t chunkPositionX;
        uint8_t chunkPositionY;
        uint32_t positionDataLocal;

        ar(chunkPositionX, positionDataLocal, directionXCompact, directionYCompact, health, animationFrame, animationFrameTickCompact, yScaleMultCompact,
            toolType, toolRotationCompact, armour, chunkViewRange, userID, bitPacked);
        
        std::vector<bool*> bitPackValues = getBitPackValues();
        for (int i = 0; i < bitPackValues.size(); i++)
        {
            *bitPackValues[i] = ((bitPacked >> i) & 0b1);
        }

        if (fishingRodCasted)
        {
            ar(fishingRodBobWorldTile.x, fishingRodBobWorldTile.y);
        }

        animationFrameTick = animationFrameTickCompact.getValue(2);
        yScaleMult = yScaleMultCompact.getValue(2);

        direction.x = directionXCompact.getValue(2);
        direction.y = directionYCompact.getValue(2);

        // Uncompress position
        chunkPositionY = (positionDataLocal >> 22) & 0xFF;
        position.x = chunkPositionX * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
        position.x += static_cast<float>(positionDataLocal & 0x7FF) / 10.0f;
        position.y = chunkPositionY * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
        position.y += static_cast<float>((positionDataLocal >> 11) & 0x7FF) / 10.0f;

        toolRotation = toolRotationCompact.getValue(2);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const override
    {
        return PacketType::PlayerCharacterInfo;
    }

private:
    inline std::vector<bool*> getBitPackValues()
    {
        std::vector<bool*> bitPackValues = {&flipped, &onWater, &inRocket, &fishingRodCasted, &fishBitingLine, &usingTool};
        return bitPackValues;
    }

    inline std::vector<const bool*> getBitPackValues() const
    {
        std::vector<const bool*> bitPackValues = {&flipped, &onWater, &inRocket, &fishingRodCasted, &fishBitingLine, &usingTool};
        return bitPackValues;
    }

};