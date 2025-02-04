#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <unordered_set>

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
#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"

struct PlayerGameSave
{
    int seed;
    PlanetType planetType = -1;
    RoomType roomDestinationType = -1;

    InventoryData inventory;
    InventoryData armourInventory;

    std::unordered_set<std::string> recipesSeen;

    float time;
    int day;

    // bool isInRoom = false;
    // uint32_t inRoomID = 0;
    // sf::Vector2f positionInRoom;

    int timePlayed = 0;

    // template <class Archive>
    // void serialize(Archive& ar)
    // {
    //     ar(seed, planetType, inventory, armourInventory, time, day);
    // }
};

// Stores items and object types saved mapped to same items / objects at current type index
struct GameDataVersionMapping
{
    std::unordered_map<ItemType, ItemType> itemTypeMap;
    std::unordered_map<ObjectType, ObjectType> objectTypeMap;
};

struct PlanetGameSave
{
    sf::Vector2f playerLastPlanetPos;

    bool isInRoom = false;
    uint32_t inRoomID = 0;
    sf::Vector2f positionInRoom;

    std::optional<ObjectReference> rocketObjectUsed = std::nullopt;

    std::vector<ChunkPOD> chunks;

    ChestDataPool chestDataPool;
    RoomPool structureRoomPool;

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        if (version <= 2)
        {
            ar(playerLastPlanetPos.x, playerLastPlanetPos.y, isInRoom, inRoomID, positionInRoom.x, positionInRoom.y, rocketObjectUsed, chunks, chestDataPool, structureRoomPool);
        }
    }

    void mapVersions(const GameDataVersionMapping& gameDataVersionMapping)
    {
        for (ChunkPOD& chunkPod : chunks)
        {
            chunkPod.mapVersions(gameDataVersionMapping.objectTypeMap);
        }

        chestDataPool.mapVersions(gameDataVersionMapping.itemTypeMap);
        structureRoomPool.mapVersions(gameDataVersionMapping.objectTypeMap);
    }
};

struct RoomDestinationGameSave
{
    Room roomDestination;
    ChestDataPool chestDataPool;

    sf::Vector2f playerLastPos;

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        if (version == 1)
        {
            ar(roomDestination, chestDataPool, playerLastPos.x, playerLastPos.y);
        }
    }

    void mapVersions(const GameDataVersionMapping& gameDataVersionMapping)
    {
        roomDestination.mapVersions(gameDataVersionMapping.objectTypeMap);
        chestDataPool.mapVersions(gameDataVersionMapping.itemTypeMap);
    }
};

CEREAL_CLASS_VERSION(PlanetGameSave, 2);
CEREAL_CLASS_VERSION(RoomDestinationGameSave, 1);

struct SaveFileSummary
{
    std::string name;
    int timePlayed = 0;
    std::string timePlayedString;
};

struct OptionsSave
{
    int musicVolume = 100;
    int controllerGlyphType = 0;
};

class GameSaveIO
{
public:
    GameSaveIO() = default;
    GameSaveIO(std::string fileName);
    
    bool loadPlayerSave(PlayerGameSave& playerGameSave);
    // bool load(PlayerGameSave& playerGameSave, PlanetGameSave& planetGameSave);
    bool loadPlanetSave(PlanetType planetType, PlanetGameSave& planetGameSave);
    bool loadRoomDestinationSave(RoomType roomDestinationType, RoomDestinationGameSave& roomDestinationGameSave);

    // bool writePlayerSave(const PlayerGameSave& playerGameSave, const PlanetGameSave& planetGameSave);
    bool writePlayerSave(const PlayerGameSave& playerGameSave);
    bool writePlanetSave(PlanetType planetType, const PlanetGameSave& planetGameSave);
    bool writeRoomDestinationSave(const RoomDestinationGameSave& roomDestinationGameSave);

    std::vector<SaveFileSummary> getSaveFiles();

    bool attemptDeleteSave();

    bool writeOptionsSave(const OptionsSave& optionsSave);
    bool loadOptionsSave(OptionsSave& optionsSave);


private:
    void createSaveDirectoryIfRequired();

    // Used to temporarily switch save file name, to be able to load all save files and create summaries
    bool loadPlayerSaveFromName(std::string fileName, PlayerGameSave& playerGameSave);

    bool loadGameDataVersionMapping(const std::string& baseFileName, GameDataVersionMapping& gameDataVersionMapping);
    bool createAndWriteGameDataVersionMapping(const std::string& baseFileName);

    std::string getPlanetGameDataVersionMappingFileName(PlanetType planetType);
    std::string getRoomDestinationGameDataVersionMappingFileName(RoomType roomDestinationType);

    std::string getRootDir();

private:
    std::string fileName;

};