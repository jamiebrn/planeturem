#pragma once

#include <optional>

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Vector.hpp"

#include "Data/typedefs.hpp"
#include "World/ChunkPosition.hpp"

struct PacketDataLandPlaced : public IPacketData
{
    PlanetType planetType;
    ChunkPosition chunk;
    pl::Vector2<uint8_t> tile;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(planetType, chunk, tile.x, tile.y);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const override
    {
        return PacketType::LandPlaced;
    }
};