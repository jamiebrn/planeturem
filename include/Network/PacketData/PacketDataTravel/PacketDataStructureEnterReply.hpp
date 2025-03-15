#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "World/ChunkPosition.hpp"
#include "World/Room.hpp"

struct PacketDataStructureEnterReply : public IPacketData
{
    PlanetType planetType;
    ChunkPosition chunkPos;

    uint32_t structureID;
    sf::Vector2f structureEntrancePos;
    RoomType roomType;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(planetType, chunkPos, structureID, structureEntrancePos.x, structureEntrancePos.y, roomType);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::StructureEnterReply;
    }
};