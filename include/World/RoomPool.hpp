#pragma once

#include <vector>
#include <unordered_map>

#include <SFML/Graphics.hpp>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>

#include "World/Room.hpp"
#include "World/ChestDataPool.hpp"

#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"

class RoomPool
{
public:
    RoomPool() = default;

    uint32_t createRoom(RoomType roomType, ChestDataPool& chestDataPool);

    Room& getRoom(uint32_t structureID);


    // Save / load
    template<class Archive>
    void serialize(Archive& archive, const std::uint32_t version)
    {
        archive(rooms);
    }

    void mapVersions(const std::unordered_map<ObjectType, ObjectType>& objectVersionMap)
    {
        for (Room& room : rooms)
        {
            room.mapVersions(objectVersionMap);
        }
    }

private:
    // 0xFFFFFFFF reserved for uninitialised room
    std::vector<Room> rooms;
};

CEREAL_CLASS_VERSION(RoomPool, 1);