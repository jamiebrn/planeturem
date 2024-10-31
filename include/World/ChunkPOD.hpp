#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <optional>
#include <unordered_map>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/array.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/optional.hpp>

#include "World/ChunkPosition.hpp"
#include "Object/BuildableObjectPOD.hpp"
#include "Object/StructureObjectPOD.hpp"
#include "Entity/EntityPOD.hpp"

#include "Data/typedefs.hpp"

struct ChunkPOD
{
    ChunkPosition chunkPosition;
    std::array<std::array<uint16_t, 8>, 8> groundTileGrid;

    std::array<std::array<std::optional<BuildableObjectPOD>, 8>, 8> objectGrid;
    std::vector<EntityPOD> entities;

    std::optional<StructureObjectPOD> structureObject;

    bool modified;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(chunkPosition.x, chunkPosition.y, groundTileGrid, objectGrid, entities, structureObject, modified);
    }

    void mapVersions(const std::unordered_map<ObjectType, ObjectType>& objectVersionMap)
    {
        for (auto& objectRow : objectGrid)
        {
            for (auto& object : objectRow)
            {
                if (!object.has_value())
                {
                    continue;
                }

                object->mapVersions(objectVersionMap);
            }
        }
    }
};