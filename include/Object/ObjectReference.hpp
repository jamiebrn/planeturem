#pragma once

#include "World/ChunkPosition.hpp"

#include <Vector.hpp>

#include <extlib/cereal/archives/binary.hpp>

#include <Core/json.hpp>

#include "Data/Serialise/Vector2Serialise.hpp"

#include "GameConstants.hpp"

struct ObjectReference
{
    ChunkPosition chunk;
    pl::Vector2<uint8_t> tile;

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        ar(chunk.x, chunk.y, tile.x, tile.y);
    }

    inline bool operator==(const ObjectReference& other) const
    {
        return (chunk.x == other.chunk.x && chunk.y == other.chunk.y && tile.x == other.tile.x && tile.y == other.tile.y);
    }

    inline pl::Vector2<uint32_t> getWorldTile() const
    {
        return pl::Vector2<uint32_t>(chunk.x * CHUNK_TILE_SIZE + tile.x, chunk.y * CHUNK_TILE_SIZE + tile.y);
    }
};

template<>
struct std::hash<ObjectReference>
{
    std::size_t operator()(const ObjectReference& objectReference) const
    {
        return std::hash<ChunkPosition>()(objectReference.chunk) ^ std::hash<int>()(objectReference.tile.x) ^ std::hash<int>()(objectReference.tile.y);
    }
};

CEREAL_CLASS_VERSION(ObjectReference, 1);

inline void from_json(const nlohmann::json& json, ObjectReference& objectReference)
{
    objectReference.chunk = json[0];
    objectReference.tile = json[1];
}

inline void to_json(nlohmann::json& json, const ObjectReference& objectReference)
{
    json[0] = objectReference.chunk;
    json[1] = objectReference.tile;
}