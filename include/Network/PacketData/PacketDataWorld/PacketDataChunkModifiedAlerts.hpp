#pragma once

#include <vector>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>

#include "Network/PacketData/PacketDataWorld/PacketDataChunkRequests.hpp"

#include "Data/typedefs.hpp"
#include "World/ChunkPosition.hpp"

struct PacketDataChunkModifiedAlerts : public PacketDataChunkRequests
{   
    inline virtual PacketType getType() const
    {
        return PacketType::ChunkModifiedAlerts;
    }
};