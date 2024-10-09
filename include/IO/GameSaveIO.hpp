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
#include <extlib/cereal/types/optional.hpp>

#include <SFML/System/Vector2.hpp>

#include "World/ChunkPOD.hpp"
#include "World/ChestDataPool.hpp"
#include "World/RoomPool.hpp"
#include "Player/InventoryData.hpp"

#include "Object/ObjectReference.hpp"

#include "Data/typedefs.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"

struct PlayerGameSave
{
    int seed;
    PlanetType planetType;

    InventoryData inventory;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(seed, planetType, inventory);
    }
};

struct PlanetGameSave
{
    sf::Vector2f playerLastPos;
    std::optional<ObjectReference> rocketObjectUsed = std::nullopt;

    std::vector<ChunkPOD> chunks;

    ChestDataPool chestDataPool;
    RoomPool structureRoomPool;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(playerLastPos.x, playerLastPos.y, rocketObjectUsed, chunks, chestDataPool, structureRoomPool);
    }
};

class GameSaveIO
{
public:
    GameSaveIO() = default;
    GameSaveIO(std::string fileName);
    
    bool load(PlayerGameSave& playerGameSave, PlanetGameSave& planetGameSave);
    bool loadPlanet(PlanetType planetType, PlanetGameSave& planetGameSave);

    bool write(const PlayerGameSave& playerGameSave, const PlanetGameSave& planetGameSave);

    std::vector<std::string> getSaveFiles();

private:
    void createSaveDirectoryIfRequired();

private:
    std::string fileName;

};