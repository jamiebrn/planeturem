#pragma once

#include <memory>
#include <vector>

#include <SFML/Graphics.hpp>

#include "World/Room.hpp"
#include "World/ChestDataPool.hpp"

#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"

class RoomPool
{
public:
    RoomPool();

    uint32_t createRoom(StructureType structureType, ChestDataPool& chestDataPool);

    Room& getRoom(uint32_t structureID);

private:
    // 0xFFFFFFFF reserved for uninitialised room
    std::unique_ptr<std::vector<std::unique_ptr<Room>>> rooms;
};