#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "World/ChunkPosition.hpp"
#include "World/WorldMap.hpp"

struct PacketDataMapChunkDiscovered : public IPacketData
{
    PlanetType planetType;
    ChunkWorldMapSection worldMapSection;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(planetType, worldMapSection);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::MapChunkDiscovered;
    }
};