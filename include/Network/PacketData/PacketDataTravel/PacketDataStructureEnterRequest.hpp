#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "World/ChunkPosition.hpp"

struct PacketDataStructureEnterRequest : public IPacketData
{
    PlanetType planetType;
    ChunkPosition chunkPos;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(planetType, chunkPos);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::StructureEnterRequest;
    }
};