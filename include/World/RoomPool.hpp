#pragma once

#include <vector>

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
    void serialize(Archive& archive)
    {
        archive(rooms);
    }

private:
    // 0xFFFFFFFF reserved for uninitialised room
    std::vector<Room> rooms;
};