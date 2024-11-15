#include "World/RoomDestinationManager.hpp"

void RoomDestinationManager::loadRoomDestinationType(RoomType roomDestination)
{
    // Load from disk through game save IO if possible
    // If not, create new room of type
}

const Room& RoomDestinationManager::getRoom()
{
    return currentRoom;
}