#include "World/RoomPool.hpp"

uint32_t RoomPool::createRoom(RoomType roomType, ChestDataPool& chestDataPool)
{
    if (std::make_signed_t<std::size_t>(rooms.size()) - 1 >= 0xFFFFFFFF)
        return 0xFFFFFFFF;
    
    rooms.emplace_back(roomType, chestDataPool);

    return (rooms.size() - 1);
}

Room& RoomPool::getRoom(uint32_t structureID)
{
    return rooms.at(structureID);
}

bool RoomPool::isIDValid(uint32_t structureID)
{
    return (structureID < rooms.size());
}