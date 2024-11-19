#pragma once

#include <SFML/Graphics.hpp>
#include "World/ChunkPosition.hpp"

#include <extlib/cereal/archives/binary.hpp>

struct ObjectReference
{
    ChunkPosition chunk;
    sf::Vector2i tile;

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        ar(chunk.x, chunk.y, tile.x, tile.y);
    }
};

CEREAL_CLASS_VERSION(ObjectReference, 1);