#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <optional>

#include "Data/typedefs.hpp"
#include "Player/InventoryData.hpp"

struct RoomObjectData
{
    ObjectType objectType;

    // Chest contents, if is chest
    std::optional<InventoryData> chestContents = std::nullopt;
};

struct RoomData
{
    sf::Vector2i tileSize;

    sf::IntRect textureRect;

    sf::Vector2i collisionBitmaskOffset;

    // Stores objects that will be placed in room based on bitmask blue channel
    std::unordered_map<uint8_t, RoomObjectData> objectsInRoom;
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