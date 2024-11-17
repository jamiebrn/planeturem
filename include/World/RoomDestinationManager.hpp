#pragma once

#include <SFML/Graphics.hpp>

#include "IO/GameSaveIO.hpp"

#include "World/Room.hpp"
#include "World/ChestDataPool.hpp"

#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"

class RoomDestinationManager
{
public:
    RoomDestinationManager() = default;

    void loadRoomDestinationType(RoomType roomDestination, ChestDataPool& chestDataPool);

    Room& getRoom();
    
private:
    Room currentRoom;

};