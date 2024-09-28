#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

#include "Data/typedefs.hpp"

struct RoomData
{
    sf::Vector2i tileSize;

    sf::IntRect textureRect;

    sf::Vector2i collisionBitmaskOffset;

    // Stores objects that will be placed in room based on bitmask blue channel
    std::unordered_map<uint8_t, ObjectType> objectsInRoom;
};

struct StructureData
{
    std::string name;

    sf::Vector2i size;

    sf::IntRect textureRect;
    sf::Vector2f textureOrigin;

    sf::Vector2i collisionBitmaskOffset;

    RoomData roomData;
};