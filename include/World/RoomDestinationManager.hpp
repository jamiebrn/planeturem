#pragma once

#include <SFML/Graphics.hpp>

#include "IO/GameSaveIO.hpp"

#include "World/Room.hpp"

#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"

class RoomDestinationManager
{
public:
    RoomDestinationManager() = default;

    void loadRoomDestinationType(RoomType roomDestination);

    const Room& getRoom();
    
private:
    Room currentRoom;

};