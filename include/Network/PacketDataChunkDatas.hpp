#pragma once

#include <vector>
#include <array>
#include <optional>
#include <unordered_map>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/array.hpp>
#include <extlib/cereal/types/optional.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/unordered_map.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "World/ChunkPosition.hpp"
#include "World/ChunkPOD.hpp"
#include "Object/BuildableObjectPOD.hpp"
#include "Object/StructureObjectPOD.hpp"
#include "Player/ItemPickup.hpp"

struct PacketDataChunkDatas : public IPacketData
{
    struct ChunkData
    {
        ChunkPosition chunkPosition;
        std::array<std::array<uint16_t, 8>, 8> groundTileGrid;
    
        std::array<std::array<std::optional<BuildableObjectPOD>, 8>, 8> objectGrid;
    
        std::optional<StructureObjectPOD> structureObject;

        std::unordered_map<uint64_t, ItemPickup> itemPickupsRelative;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(chunkPosition.x, chunkPosition.y, groundTileGrid, objectGrid, structureObject);
        }

        void setFromPOD(const ChunkPOD& pod)
        {
            chunkPosition = pod.chunkPosition;
            groundTileGrid = pod.groundTileGrid;
            objectGrid = pod.objectGrid;
            structureObject = pod.structureObject;
        }

        ChunkPOD createPOD() const
        {
            ChunkPOD pod;
            pod.chunkPosition = chunkPosition;
            pod.groundTileGrid = groundTileGrid;
            pod.objectGrid = objectGrid;
            pod.structureObject = structureObject;
            return pod;
        }
    };

    std::vector<ChunkData> chunkDatas;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(chunkDatas);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const
    {
        return PacketType::ChunkDatas;
    }
};