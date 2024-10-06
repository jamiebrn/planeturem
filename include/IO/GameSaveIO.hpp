#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <filesystem>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <SFML/System/Vector2.hpp>

#include "World/ChunkPOD.hpp"
#include "World/ChestDataPool.hpp"
#include "World/RoomPool.hpp"
#include "Player/InventoryData.hpp"

#include "Data/typedefs.hpp"

struct GameSave
{
    int seed;
    sf::Vector2f playerPos;
    InventoryData inventory;

    std::vector<ChunkPOD> chunks;

    PlanetType planetType;

    ChestDataPool chestDataPool;
    RoomPool structureRoomPool;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(seed, planetType, playerPos.x, playerPos.y, inventory, chunks, chestDataPool, structureRoomPool);
    }
};

class GameSaveIO
{
public:
    GameSaveIO(std::string fileName);
    
    bool load(GameSave& gameSave);
    bool write(const GameSave& gameSave);

private:
    std::string fileName;

};