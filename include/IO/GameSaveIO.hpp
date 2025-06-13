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

#include <extlib/lzav.h>

#include <platform_folders.h>

#include <Core/json.hpp>

#include "Core/InputManager.hpp"

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

#include "Player/PlayerData.hpp"

struct PlayerGameSave
{
    int seed;

    PlayerData playerData;

    std::unordered_map<uint64_t, PlayerData> networkPlayerDatas;

    float time;
    int day;

    int timePlayed = 0;
};

// Stores current state of item and object types, mapped to their respective names
struct GameDataVersionState
{
    inline GameDataVersionState()
    {
        itemNameTypeMap = ItemDataLoader::getItemNameToTypeMap();
        objectNameTypeMap = ObjectDataLoader::getObjectNameToTypeMap();
    }
    
    std::unordered_map<std::string, ItemType> itemNameTypeMap;
    std::unordered_map<std::string, ObjectType> objectNameTypeMap;
    
    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        ar(itemNameTypeMap, objectNameTypeMap);
    }
};

CEREAL_CLASS_VERSION(GameDataVersionState, 1);

// Stores items and object types saved mapped to same items / objects at current type index
struct GameDataVersionMapping
{
    inline GameDataVersionMapping(const GameDataVersionState& versionState)
    {
        for (auto iter = versionState.itemNameTypeMap.begin(); iter != versionState.itemNameTypeMap.end(); iter++)
        {
            itemTypeMap[iter->second] = ItemDataLoader::getItemTypeFromName(iter->first);
        }

        for (auto iter = versionState.objectNameTypeMap.begin(); iter != versionState.objectNameTypeMap.end(); iter++)
        {
            objectTypeMap[iter->second] = ObjectDataLoader::getObjectTypeFromName(iter->first);
        }
    }

    std::unordered_map<ItemType, ItemType> itemTypeMap;
    std::unordered_map<ObjectType, ObjectType> objectTypeMap;
};

struct PlanetGameSave
{
    std::vector<ChunkPOD> chunks;

    ChestDataPool chestDataPool;
    RoomPool structureRoomPool;

    GameDataVersionState versionState;

    template <class Archive>
    void save(Archive& ar, const std::uint32_t version) const
    {
        if (version >= 4)
        {
            ar(versionState, chunks, chestDataPool, structureRoomPool);
        }
    }

    template <class Archive>
    void load(Archive& ar, const std::uint32_t version)
    {
        if (version >= 4)
        {
            ar(versionState, chunks, chestDataPool, structureRoomPool);
        }

        GameDataVersionMapping versionMapping(versionState);
        mapVersions(versionMapping);
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

    // sf::Vector2f playerLastPos;

    GameDataVersionState versionState;

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        // if (version == 1)
        // {
        //     ar(roomDestination, chestDataPool, playerLastPos.x, playerLastPos.y);
        // }
        if (version == 3)
        {
            ar(versionState, roomDestination, chestDataPool);
        }

        GameDataVersionMapping versionMapping(versionState);
        mapVersions(versionMapping);
        
        roomDestination.clearUnusedChestIDs(chestDataPool);
    }

    void mapVersions(const GameDataVersionMapping& gameDataVersionMapping)
    {
        roomDestination.mapVersions(gameDataVersionMapping.objectTypeMap);
        chestDataPool.mapVersions(gameDataVersionMapping.itemTypeMap);
    }
};

CEREAL_CLASS_VERSION(PlanetGameSave, 4);
CEREAL_CLASS_VERSION(RoomDestinationGameSave, 3);

struct SaveFileSummary
{
    std::string name;
    std::string playerName;
    int timePlayed = 0;
    std::string timePlayedString;

    PlayerData playerData;
};

struct OptionsSave
{
    int musicVolume = 30;
    int soundVolume = 80;
    bool screenShakeEnabled = true;
    int controllerGlyphType = 0;
    bool vSync = true;
};

struct CompressedData
{
    CompressedData() = default;
    CompressedData(const std::vector<char> uncompressedData);
    std::vector<char> decompress();

    std::vector<char> data;
    uint32_t uncompressedSize;

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        ar(data, uncompressedSize);
    }
};

CEREAL_CLASS_VERSION(CompressedData, 1);

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

    bool writeInputBindingsSave(const InputBindingsSave& inputBindingsSave);
    bool loadInputBindingsSave(InputBindingsSave& inputBindingsSave);

private:
    void createSaveDirectoryIfRequired();

    // Used to temporarily switch save file name, to be able to load all save files and create summaries
    bool loadPlayerSaveFromName(std::string fileName, PlayerGameSave& playerGameSave);

    // bool loadGameDataVersionMapping(const std::string& baseFileName, GameDataVersionMapping& gameDataVersionMapping);
    // bool createAndWriteGameDataVersionMapping(const std::string& baseFileName);

    // std::string getPlanetGameDataVersionMappingFileName(PlanetType planetType);
    // std::string getRoomDestinationGameDataVersionMappingFileName(RoomType roomDestinationType);

    std::string getRootDir();

private:
    std::string fileName;

};