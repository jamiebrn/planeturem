#pragma once

#include <vector>
#include <unordered_map>

#include <SFML/Graphics.hpp>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/unordered_map.hpp>

#include "World/Room.hpp"
#include "World/ChestDataPool.hpp"

#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"

class RoomPool
{
public:
    RoomPool() = default;

    uint32_t createRoom(RoomType roomType, ChestDataPool& chestDataPool);

    void overwriteRoomData(uint32_t id, const Room& room);

    Room& getRoom(uint32_t structureID);

    bool isIDValid(uint32_t structureID);


    // Save / load
    template<class Archive>
    void serialize(Archive& archive, const std::uint32_t version)
    {
        if (version == 2)
        {
            archive(rooms);
        }
    }

    void mapVersions(const std::unordered_map<ObjectType, ObjectType>& objectVersionMap)
    {
        for (auto& roomPair : rooms)
        {
            roomPair.second.mapVersions(objectVersionMap);
        }
    }

private:
    // 0xFFFFFFFF reserved for uninitialised room
    std::unordered_map<uint32_t, Room> rooms;
    uint32_t topDataSlot = 0;

};

CEREAL_CLASS_VERSION(RoomPool, 2);