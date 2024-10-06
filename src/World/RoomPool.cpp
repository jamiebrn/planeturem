#include "World/RoomPool.hpp"

uint32_t RoomPool::createRoom(StructureType structureType, ChestDataPool& chestDataPool)
{
    if (std::make_signed_t<std::size_t>(rooms.size()) - 1 >= 0xFFFFFFFF)
        return 0xFFFFFFFF;
    
    const RoomData& roomData = StructureDataLoader::getStructureData(structureType).roomData;
    
    rooms.emplace_back(roomData, chestDataPool);

    return (rooms.size() - 1);
}

Room& RoomPool::getRoom(uint32_t structureID)
{
    return rooms.at(structureID);
}