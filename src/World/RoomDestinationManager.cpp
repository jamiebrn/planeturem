#include "World/RoomDestinationManager.hpp"

void RoomDestinationManager::loadRoomDestinationType(RoomType roomDestination, ChestDataPool& chestDataPool)
{
    // TODO:
    // Load from disk through game save IO if possible
    // If not, create new room of type

    currentRoom = Room(roomDestination, chestDataPool);
}

Room& RoomDestinationManager::getRoom()
{
    return currentRoom;
}