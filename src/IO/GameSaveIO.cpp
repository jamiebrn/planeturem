#include "IO/GameSaveIO.hpp"

GameSaveIO::GameSaveIO(std::string fileName)
{
    this->fileName = fileName;
}

// bool GameSaveIO::load(PlayerGameSave& playerGameSave, PlanetGameSave& planetGameSave)
// {
//     createSaveDirectoryIfRequired();

//     std::filesystem::path dir(getRootDir() + "Saves/" + fileName + "/");

//     if (!std::filesystem::exists(dir))
//     {
//         return false;
//     }

//     try
//     {
//         // Load player file
//         if (!loadPlayerSave(playerGameSave))
//         {
//             return false;
//         }

//         // Load planet file
//         if (!loadPlanet(playerGameSave.planetType, planetGameSave))
//         {
//             throw std::invalid_argument("Could not open planet file for \"" + fileName + "\"");
//         }

//         return true;
//     }
//     catch(const std::exception& e)
//     {
//         std::cerr << e.what() << '\n';
//         return false;
//     }

//     return false;
// }

bool GameSaveIO::loadPlayerSave(PlayerGameSave& playerGameSave)
{
    createSaveDirectoryIfRequired();

    std::filesystem::path dir(getRootDir() + "Saves/" + fileName + "/");

    if (!std::filesystem::exists(dir))
    {
        return false;
    }

    std::fstream in(getRootDir() + "Saves/" + fileName + "/Player.dat", std::ios::in);

    if (!in)
    {
        return false;
    }

    try
    {
        nlohmann::json json = nlohmann::json::parse(in);
        playerGameSave.seed = json["seed"];
        playerGameSave.inventory = json["inventory"];
        playerGameSave.armourInventory = json["armour-inventory"];
        playerGameSave.time = json["time"];
        playerGameSave.day = json["day"];

        if (json.contains("planet"))
        {
            playerGameSave.planetType = PlanetGenDataLoader::getPlanetTypeFromName(json["planet"]);
        }
        else if (json.contains("roomdest"))
        {
            playerGameSave.roomDestinationType = StructureDataLoader::getRoomTypeTravelLocationFromName(json["roomdest"]);
        }
        else
        {
            const std::string& defaultPlanetName = PlanetGenDataLoader::getPlanetGenData(0).name;
            std::cout << "Error: Player file \"" + fileName + "\" has no previous location. Defaulting to planet \"" + defaultPlanetName + "\"\n";
            playerGameSave.planetType = 0;
        }

        // if (json.contains("in-room"))
        // {
        //     playerGameSave.isInRoom = true;
        //     playerGameSave.inRoomID = json.at("room-id");
        //     playerGameSave.positionInRoom = json.at("room-player-pos");
        // }

        if (json.contains("time-played"))
        {
            playerGameSave.timePlayed = json.at("time-played");
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;   
}

bool GameSaveIO::loadPlanetSave(PlanetType planetType, PlanetGameSave& planetGameSave)
{
    try
    {
        const std::string& planetName = PlanetGenDataLoader::getPlanetGenData(planetType).name;

        std::fstream in(getRootDir() + "Saves/" + fileName + "/" + planetName + ".dat", std::ios::in | std::ios::binary);
        
        if (!in)
        {
            return false;
        }

        cereal::BinaryInputArchive archive(in);

        archive(planetGameSave);

        GameDataVersionMapping gameDataVersionMapping;

        if (!loadGameDataVersionMapping(getPlanetGameDataVersionMappingFileName(planetType), gameDataVersionMapping))
        {
            return false;
        }

        // Map loaded planet data to new types to prevent saves breaking
        planetGameSave.mapVersions(gameDataVersionMapping);

        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    
    return false;
}

bool GameSaveIO::loadRoomDestinationSave(RoomType roomDestinationType, RoomDestinationGameSave& roomDestinationGameSave)
{
    try
    {
        const std::string& roomDestinationName = StructureDataLoader::getRoomData(roomDestinationType).name;

        std::fstream in(getRootDir() + "Saves/" + fileName + "/" + roomDestinationName + "-roomdest" + ".dat", std::ios::in | std::ios::binary);
        
        if (!in)
        {
            return false;
        }

        cereal::BinaryInputArchive archive(in);

        archive(roomDestinationGameSave);

        GameDataVersionMapping gameDataVersionMapping;

        if (!loadGameDataVersionMapping(getRoomDestinationGameDataVersionMappingFileName(roomDestinationType), gameDataVersionMapping))
        {
            return false;
        }

        // Map loaded room destination data to new types to prevent saves breaking
        roomDestinationGameSave.mapVersions(gameDataVersionMapping);

        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    
    return false;
}

// bool GameSaveIO::write(const PlayerGameSave& playerGameSave, const PlanetGameSave& planetGameSave)
// {
    // createSaveDirectoryIfRequired();

    // try
    // {
    //     // Save player file
    //     if (!writePlayerSave(playerGameSave))
    //     {
    //         throw std::invalid_argument("Could not open player file for \"" + fileName + "\"");
    //     }

        // Save planet file
    //     {
    //         const std::string& planetName = PlanetGenDataLoader::getPlanetGenData(playerGameSave.planetType).name;

    //         std::fstream out(getRootDir() + "Saves/" + fileName + "/" + planetName + ".dat", std::ios::out | std::ios::binary);

    //         if (!out || !createAndWriteGameDataVersionMapping(getPlanetGameDataVersionMappingFileName(playerGameSave.planetType)))
    //         {
    //             throw std::invalid_argument("Could not open planet file for \"" + fileName + "\"");
    //         }

    //         cereal::BinaryOutputArchive archive(out);

    //         archive(planetGameSave);
    //     }

    //     return true;
    // }
    // catch(const std::exception& e)
    // {
    //     std::cerr << e.what() << '\n';
    //     return false;
    // }
    
    // return false;
// }

bool GameSaveIO::writePlayerSave(const PlayerGameSave& playerGameSave)
{
    createSaveDirectoryIfRequired();

    std::fstream out(getRootDir() + "Saves/" + fileName + "/Player.dat", std::ios::out);

    if (!out)
    {
        return false;
    }

    try
    {
        nlohmann::json json;
        json["seed"] = playerGameSave.seed;
        json["inventory"] = playerGameSave.inventory;
        json["armour-inventory"] = playerGameSave.armourInventory;
        json["time"] = playerGameSave.time;
        json["day"] = playerGameSave.day;

        if (playerGameSave.planetType >= 0)
        {
            json["planet"] = PlanetGenDataLoader::getPlanetGenData(playerGameSave.planetType).name;
        }
        else if (playerGameSave.roomDestinationType >= 0)
        {
            json["roomdest"] = StructureDataLoader::getRoomData(playerGameSave.roomDestinationType).name;
        }
        
        // if (playerGameSave.isInRoom)
        // {
        //     json["in-room"] = true;
        //     json["room-id"] = playerGameSave.inRoomID;
        //     json["room-player-pos"] = playerGameSave.positionInRoom;
        // }

        json["time-played"] = playerGameSave.timePlayed;

        out << json;
        out.close();

        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

bool GameSaveIO::writePlanetSave(PlanetType planetType, const PlanetGameSave& planetGameSave)
{
    createSaveDirectoryIfRequired();

    try
    {
        const std::string& planetName = PlanetGenDataLoader::getPlanetGenData(planetType).name;

        std::fstream out(getRootDir() + "Saves/" + fileName + "/" + planetName + ".dat", std::ios::out | std::ios::binary);

        if (!out || !createAndWriteGameDataVersionMapping(getPlanetGameDataVersionMappingFileName(planetType)))
        {
            throw std::invalid_argument("Could not open planet \"" + planetName + "\" file for \"" + fileName + "\"");
        }

        cereal::BinaryOutputArchive archive(out);

        archive(planetGameSave);

        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return false;
}

bool GameSaveIO::writeRoomDestinationSave(const RoomDestinationGameSave& roomDestinationGameSave)
{
    createSaveDirectoryIfRequired();

    try
    {
        RoomType roomDestinationType = roomDestinationGameSave.roomDestination.getRoomType();

        const std::string& roomDestinationName = StructureDataLoader::getRoomData(roomDestinationType).name;

        std::fstream out(getRootDir() + "Saves/" + fileName + "/" + roomDestinationName + "-roomdest" + ".dat", std::ios::out | std::ios::binary);

        if (!out || !createAndWriteGameDataVersionMapping(getRoomDestinationGameDataVersionMappingFileName(roomDestinationType)))
        {
            throw std::invalid_argument("Could not open room \"" + roomDestinationName + "\" file for \"" + fileName + "\"");
        }

        cereal::BinaryOutputArchive archive(out);

        archive(roomDestinationGameSave);

        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return false;
}

bool GameSaveIO::loadPlayerSaveFromName(std::string fileName, PlayerGameSave& playerGameSave)
{
    std::string previousFileName = this->fileName;
    this->fileName = fileName;
    
    bool success = loadPlayerSave(playerGameSave);
    this->fileName = previousFileName;

    return success;
}

std::vector<SaveFileSummary> GameSaveIO::getSaveFiles()
{
    createSaveDirectoryIfRequired();

    struct SaveFileWithDate
    {
        std::string name;
        std::filesystem::file_time_type lastModified;
    };

    std::vector<SaveFileWithDate> saveFilesWithDate;

    for (const auto& saveFile : std::filesystem::directory_iterator(getRootDir() + "Saves"))
    {
        if (!saveFile.is_directory())
            continue;
        
        SaveFileWithDate saveFileWithDate;

        saveFileWithDate.name = saveFile.path().filename().generic_string();

        saveFileWithDate.lastModified =  saveFile.last_write_time();

        saveFilesWithDate.push_back(saveFileWithDate);
    }
    
    std::vector<SaveFileSummary> saveFiles;

    // Sort save files by date
    std::sort(saveFilesWithDate.begin(), saveFilesWithDate.end(), [](const SaveFileWithDate& saveA, const SaveFileWithDate& saveB)
    {
        return (saveA.lastModified > saveB.lastModified);
    });

    for (const SaveFileWithDate& saveFileWithDate : saveFilesWithDate)
    {
        SaveFileSummary saveFileSummary;
        saveFileSummary.name = saveFileWithDate.name;

        // Load time spent playing
        PlayerGameSave playerSave;
        if (!loadPlayerSaveFromName(saveFileSummary.name, playerSave))
        {
            std::cout << "Error - could not load player save " + saveFileSummary.name << "\n";
            continue;
        }

        saveFileSummary.timePlayed = playerSave.timePlayed;

        // Convert to string
        int seconds = playerSave.timePlayed;
        int minutes = seconds / 60;
        int hours = minutes / 60;
        seconds %= 60;
        minutes %= 60;

        std::string timePlayedString;
        if (hours > 0)
        {
            timePlayedString += std::to_string(hours) + ":";
        }
        std::vector<std::string> timeStrings = {std::to_string(minutes), std::to_string(seconds)};
        for (std::string& string : timeStrings)
        {
            if (string.size() < 2)
            {
                string = "0" + string;
            }
        }
        timePlayedString += timeStrings[0] + ":" + timeStrings[1];

        saveFileSummary.timePlayedString = timePlayedString;

        saveFiles.push_back(saveFileSummary);
    }

    return saveFiles;
}

void GameSaveIO::createSaveDirectoryIfRequired()
{
    std::filesystem::path dir(sago::getDataHome() + "/Planeturem");
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }

    dir = std::filesystem::path(sago::getDataHome() + "/Planeturem/Saves");
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }

    if (fileName.empty())
    {
        return;
    }

    dir = std::filesystem::path(getRootDir() + "Saves/" + fileName + "/");
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }
}

bool GameSaveIO::loadGameDataVersionMapping(const std::string& baseFileName, GameDataVersionMapping& gameDataVersionMapping)
{
    std::fstream in(getRootDir() + "Saves/" + fileName + "/version/" + baseFileName, std::ios::in);

    if (!in)
    {
        return false;
    }

    try
    {
        nlohmann::json json = nlohmann::json::parse(in);

        std::unordered_map<std::string, ItemType> itemNameToTypeMap = json["items"];
        std::unordered_map<std::string, ObjectType> objectNameToTypeMap = json["objects"];

        for (auto iter = itemNameToTypeMap.begin(); iter != itemNameToTypeMap.end(); iter++)
        {
            gameDataVersionMapping.itemTypeMap[iter->second] = ItemDataLoader::getItemTypeFromName(iter->first);
        }

        for (auto iter = objectNameToTypeMap.begin(); iter != objectNameToTypeMap.end(); iter++)
        {
            gameDataVersionMapping.objectTypeMap[iter->second] = ObjectDataLoader::getObjectTypeFromName(iter->first);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

bool GameSaveIO::createAndWriteGameDataVersionMapping(const std::string& baseFileName)
{
    createSaveDirectoryIfRequired();

    std::filesystem::path dir(getRootDir() + "Saves/" + fileName + "/version");
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }
    
    std::ofstream f(getRootDir() + "Saves/" + fileName + "/version/" + baseFileName);
    
    if (!f)
    {
        return false;
    }
    
    try
    {
        nlohmann::json json;

        // Save all item / object etc name -> type maps to json
        json["items"] = ItemDataLoader::getItemNameToTypeMap();
        json["objects"] = ObjectDataLoader::getObjectNameToTypeMap();

        f << json;
        f.close();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return false;
    }

    return true;
}

std::string GameSaveIO::getPlanetGameDataVersionMappingFileName(PlanetType planetType)
{
    std::string planetName = PlanetGenDataLoader::getPlanetGenData(planetType).name;
    return (planetName + "VersionMapping" + ".ver");
}

std::string GameSaveIO::getRoomDestinationGameDataVersionMappingFileName(RoomType roomDestinationType)
{
    std::string roomDestinationName = StructureDataLoader::getRoomData(roomDestinationType).name;
    return (roomDestinationName + "-roomdestVersionMapping.ver");   
}

std::string GameSaveIO::getRootDir()
{
    return (sago::getDataHome() + "/Planeturem/");
}