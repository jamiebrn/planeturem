#pragma once

#include <SFML/Graphics.hpp>
#include "World/ChunkPosition.hpp"

#include <extlib/cereal/archives/binary.hpp>

struct ObjectReference
{
    ChunkPosition chunk;
    sf::Vector2i tile;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(chunk.x, chunk.y, tile.x, tile.y);
    }
};