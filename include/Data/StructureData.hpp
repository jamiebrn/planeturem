#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <optional>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/unordered_map.hpp>
#include <extlib/cereal/types/optional.hpp>

#include "Data/typedefs.hpp"
#include "Player/InventoryData.hpp"

struct RoomObjectData
{
    ObjectType objectType;

    // Chest contents, if is chest
    std::optional<std::vector<InventoryData>> chestContents = std::nullopt;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(objectType, chestContents);
    }
};

struct RoomData
{
    sf::Vector2i tileSize;

    sf::IntRect textureRect;

    sf::Vector2i collisionBitmaskOffset;

    // Stores objects that will be placed in room based on bitmask blue channel
    std::unordered_map<uint8_t, RoomObjectData> objectsInRoom;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(tileSize.x, tileSize.y, textureRect.left, textureRect.top, textureRect.width, textureRect.height, collisionBitmaskOffset.x, collisionBitmaskOffset.y, objectsInRoom);
    }
};

struct StructureData
{
    std::string name;

    sf::Vector2i size;

    sf::IntRect textureRect;
    sf::Vector2f textureOrigin;

    sf::Vector2i collisionBitmaskOffset;

    // RoomData roomData;
    RoomType roomType;
};