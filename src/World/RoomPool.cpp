#include "World/RoomPool.hpp"

uint32_t RoomPool::createRoom(RoomType roomType, ChestDataPool& chestDataPool)
{
    if (std::make_signed_t<std::size_t>(rooms.size()) - 1 >= 0xFFFFFFFF)
        return 0xFFFFFFFF;
    
    rooms[topDataSlot] = Room(roomType, &chestDataPool);

    return topDataSlot++;
}

void RoomPool::overwriteRoomData(uint32_t id, const Room& room)
{
    rooms[id] = room;
}

Room& RoomPool::getRoom(uint32_t structureID)
{
    return rooms.at(structureID);
}

bool RoomPool::isIDValid(uint32_t structureID)
{
    return rooms.contains(structureID);
}