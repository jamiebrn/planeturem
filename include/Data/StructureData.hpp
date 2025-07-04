#pragma once


#include <string>
#include <unordered_map>
#include <optional>

#include <Vector.hpp>
#include <Rect.hpp>

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

    // template <class Archive>
    // void serialize(Archive& ar)
    // {
    //     ar(objectType, chestContents);
    // }
};

struct RoomData
{
    pl::Vector2<int> tileSize;

    pl::Rect<int> textureRect;

    pl::Vector2<int> collisionBitmaskOffset;

    std::string name;
    std::string displayName;
    bool isTravelLocation = false;

    // Stores objects that will be placed in room based on bitmask blue channel
    std::unordered_map<uint8_t, RoomObjectData> objectsInRoom;

    std::string achievementUnlockOnTravel;
};

struct StructureData
{
    std::string name;

    pl::Vector2<int> size;

    pl::Rect<int> textureRect;
    pl::Vector2f textureOrigin;

    pl::Vector2<int> collisionBitmaskOffset;
    pl::Vector2<int> lightBitmaskOffset;

    // RoomData roomData;
    RoomType roomType;
};