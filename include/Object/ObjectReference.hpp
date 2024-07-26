#pragma once

#include <SFML/Graphics.hpp>
#include "World/ChunkPosition.hpp"

struct ObjectReference
{
    ChunkPosition chunk;
    sf::Vector2i tile;
};