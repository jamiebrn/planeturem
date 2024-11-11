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

#include <platform_folders.h>

#include <Core/json.hpp>

#include <SFML/System/Vector2.hpp>

#include "World/ChunkPOD.hpp"
#include "World/ChestDataPool.hpp"
#include "World/RoomPool.hpp"
#include "Player/InventoryData.hpp"

#include "Object/ObjectReference.hpp"

#include "Data/Serialise/Vector2Serialise.hpp"
#include "Data/typedefs.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"

struct PlayerGameSave
{
    int seed;
    PlanetType planetType;

    InventoryData inventory;
    InventoryData armourInventory;

    float time;
    int day;

    bool isInRoom = false;
    uint32_t inRoomID = 0;
    sf::Vector2f positionInRoom;

    int timePlayed = 0;

    // template <class Archive>
    // void serialize(Archive& ar)
    // {
    //     ar(seed, planetType, inventory, armourInventory, time, day);
    // }
};

// Stores items and object types saved mapped to same items / objects at current type index
struct PlanetDataVersionMapping
{
    std::unordered_map<ItemType, ItemType> itemTypeMap;
    std::unordered_map<ObjectType, ObjectType> objectTypeMap;
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

    void mapVersions(const PlanetDataVersionMapping& planetDataVersionMapping)
    {
        for (ChunkPOD& chunkPod : chunks)
        {
            chunkPod.mapVersions(planetDataVersionMapping.objectTypeMap);
        }

        chestDataPool.mapVersions(planetDataVersionMapping.itemTypeMap);
        structureRoomPool.mapVersions(planetDataVersionMapping.objectTypeMap);
    }
};

struct SaveFileSummary
{
    std::string name;
    int timePlayed = 0;
    std::string timePlayedString;
};

class GameSaveIO
{
public:
    GameSaveIO() = default;
    GameSaveIO(std::string fileName);
    
    bool load(PlayerGameSave& playerGameSave, PlanetGameSave& planetGameSave);
    bool loadPlanet(PlanetType planetType, PlanetGameSave& planetGameSave);

    bool write(const PlayerGameSave& playerGameSave, const PlanetGameSave& planetGameSave);

    std::vector<SaveFileSummary> getSaveFiles();

private:
    void createSaveDirectoryIfRequired();

    bool loadPlayerSave(PlayerGameSave& playerGameSave);
    bool loadPlayerSaveFromName(std::string fileName, PlayerGameSave& playerGameSave);
    bool writePlayerSave(const PlayerGameSave& playerGameSave);

    bool loadPlanetDataVersionMapping(PlanetType planetType, PlanetDataVersionMapping& planetDataVersionMapping);
    bool buildPlanetDataVersionMapping(PlanetType planetType);

    std::string getPlanetDataVersionMappingFileName(PlanetType planetType);

    std::string getRootDir();

private:
    std::string fileName;

};