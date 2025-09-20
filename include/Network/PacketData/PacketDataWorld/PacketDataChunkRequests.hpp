#pragma once

#include <vector>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "World/ChunkPosition.hpp"

struct PacketDataChunkRequests : public IPacketData
{
    PlanetType planetType;
    std::vector<ChunkPosition> chunkRequests;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(planetType, chunkRequests);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const override
    {
        return PacketType::ChunkRequests;
    }
};