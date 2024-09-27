#include "World/RoomPool.hpp"

RoomPool::RoomPool()
{
    rooms = std::make_unique<std::vector<std::unique_ptr<Room>>>();
}

uint32_t RoomPool::createRoom(StructureType structureType)
{
    if (std::make_signed_t<std::size_t>(rooms->size()) - 1 >= 0xFFFFFFFF)
        return 0xFFFFFFFF;
    
    const RoomData& roomData = StructureDataLoader::getStructureData(structureType).roomData;
    
    rooms->push_back(std::make_unique<Room>(roomData));

    return rooms->size() - 1;
}

Room& RoomPool::getRoom(uint32_t structureID)
{
    return *(rooms->at(structureID).get());
}