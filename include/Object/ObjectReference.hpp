#pragma once

// #include <SFML/Graphics.hpp>
#include "World/ChunkPosition.hpp"

#include <Vector.hpp>

#include <extlib/cereal/archives/binary.hpp>

struct ObjectReference
{
    ChunkPosition chunk;
    pl::Vector2<uint8_t> tile;

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        ar(chunk.x, chunk.y, tile.x, tile.y);
    }

    bool operator==(const ObjectReference& other) const
    {
        return (chunk.x == other.chunk.x && chunk.y == other.chunk.y && tile.x == other.tile.x && tile.y == other.tile.y);
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