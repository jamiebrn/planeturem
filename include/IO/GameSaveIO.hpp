#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <filesystem>
#include <stdexcept>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <SFML/System/Vector2.hpp>

#include "World/ChunkPOD.hpp"
#include "World/ChestDataPool.hpp"
#include "World/RoomPool.hpp"
#include "Player/InventoryData.hpp"

#include "Data/typedefs.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"

struct PlayerGameSave
{
    int seed;
    PlanetType planetType;

    sf::Vector2f playerPos;
    InventoryData inventory;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(seed, planetType, playerPos.x, playerPos.y, inventory);
    }
};

struct PlanetGameSave
{
    std::vector<ChunkPOD> chunks;

    ChestDataPool chestDataPool;
    RoomPool structureRoomPool;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(chunks, chestDataPool, structureRoomPool);
    }
};

class GameSaveIO
{
public:
    GameSaveIO(std::string fileName);
    
    bool load(PlayerGameSave& playerGameSave, PlanetGameSave& planetGameSave);
    bool write(const PlayerGameSave& playerGameSave, const PlanetGameSave& planetGameSave);

private:
    std::string fileName;

};