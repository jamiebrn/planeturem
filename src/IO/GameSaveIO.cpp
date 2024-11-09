#include "IO/GameSaveIO.hpp"

GameSaveIO::GameSaveIO(std::string fileName)
{
    this->fileName = fileName;
}

bool GameSaveIO::load(PlayerGameSave& playerGameSave, PlanetGameSave& planetGameSave)
{
    createSaveDirectoryIfRequired();

    std::filesystem::path dir(getRootDir() + "Saves/" + fileName + "/");

    if (!std::filesystem::exists(dir))
    {
        return false;
    }

    try
    {
        // Load player file
        if (!loadPlayerSave(playerGameSave))
        {
            return false;
        }

        // Load planet file
        if (!loadPlanet(playerGameSave.planetType, planetGameSave))
        {
            throw std::invalid_argument("Could not open planet file for \"" + fileName + "\"");
        }

        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return false;
}

bool GameSaveIO::loadPlanet(PlanetType planetType, PlanetGameSave& planetGameSave)
{
    const std::string& planetName = PlanetGenDataLoader::getPlanetGenData(planetType).name;

    std::fstream in(getRootDir() + "Saves/" + fileName + "/" + planetName + ".dat", std::ios::in | std::ios::binary);
    
    if (!in)
    {
        return false;
    }

    cereal::BinaryInputArchive archive(in);

    archive(planetGameSave);

    PlanetDataVersionMapping planetDataVersionMapping;

    if (!loadPlanetDataVersionMapping(planetType, planetDataVersionMapping))
    {
        return false;
    }

    // Map loaded planet data to new types to prevent saves breaking
    planetGameSave.mapVersions(planetDataVersionMapping);

    return true;
}

bool GameSaveIO::write(const PlayerGameSave& playerGameSave, const PlanetGameSave& planetGameSave)
{
    createSaveDirectoryIfRequired();

    std::filesystem::path dir(getRootDir() + "Saves/" + fileName + "/");
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }

    try
    {
        // Save player file
        if (!writePlayerSave(playerGameSave))
        {
            throw std::invalid_argument("Could not open player file for \"" + fileName + "\"");
        }

        // Save planet file
        {
            const std::string& planetName = PlanetGenDataLoader::getPlanetGenData(playerGameSave.planetType).name;

            std::fstream out(getRootDir() + "Saves/" + fileName + "/" + planetName + ".dat", std::ios::out | std::ios::binary);

            if (!out || !buildPlanetDataVersionMapping(playerGameSave.planetType))
            {
                throw std::invalid_argument("Could not open planet file for \"" + fileName + "\"");
            }

            cereal::BinaryOutputArchive archive(out);

            archive(planetGameSave);
        }

        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    
    return false;
}

bool GameSaveIO::loadPlayerSave(PlayerGameSave& playerGameSave)
{
    std::fstream in(getRootDir() + "Saves/" + fileName + "/Player.dat", std::ios::in);

    if (!in)
    {
        return false;
    }

    try
    {
        nlohmann::json json = nlohmann::json::parse(in);
        playerGameSave.seed = json["seed"];
        playerGameSave.planetType = PlanetGenDataLoader::getPlanetTypeFromName(json["planet"]);
        playerGameSave.inventory = json["inventory"];
        playerGameSave.armourInventory = json["armour-inventory"];
        playerGameSave.time = json["time"];
        playerGameSave.day = json["day"];

        if (json.contains("in-room"))
        {
            playerGameSave.isInRoom = true;
            playerGameSave.inRoomID = json.at("room-id");
            playerGameSave.positionInRoom = json.at("room-player-pos");
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;   
}

bool GameSaveIO::writePlayerSave(const PlayerGameSave& playerGameSave)
{
    std::fstream out(getRootDir() + "Saves/" + fileName + "/Player.dat", std::ios::out);

    if (!out)
    {
        return false;
    }

    nlohmann::json json;
    json["seed"] = playerGameSave.seed;
    json["planet"] = PlanetGenDataLoader::getPlanetGenData(playerGameSave.planetType).name;
    json["inventory"] = playerGameSave.inventory;
    json["armour-inventory"] = playerGameSave.armourInventory;
    json["time"] = playerGameSave.time;
    json["day"] = playerGameSave.day;

    if (playerGameSave.isInRoom)
    {
        json["in-room"] = true;
        json["room-id"] = playerGameSave.inRoomID;
        json["room-player-pos"] = playerGameSave.positionInRoom;
    }

    out << json;
    out.close();

    return true;
}

std::vector<std::string> GameSaveIO::getSaveFiles()
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
    
    std::vector<std::string> saveFiles;

    // Sort save files by date
    std::sort(saveFilesWithDate.begin(), saveFilesWithDate.end(), [](const SaveFileWithDate& saveA, const SaveFileWithDate& saveB)
    {
        return (saveA.lastModified > saveB.lastModified);
    });

    for (const SaveFileWithDate& saveFileWithDate : saveFilesWithDate)
    {
        saveFiles.push_back(saveFileWithDate.name);
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
}

bool GameSaveIO::loadPlanetDataVersionMapping(PlanetType planetType, PlanetDataVersionMapping& planetDataVersionMapping)
{
    std::fstream in(getPlanetDataVersionMappingFileName(planetType), std::ios::in);

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
            planetDataVersionMapping.itemTypeMap[iter->second] = ItemDataLoader::getItemTypeFromName(iter->first);
        }

        for (auto iter = objectNameToTypeMap.begin(); iter != objectNameToTypeMap.end(); iter++)
        {
            planetDataVersionMapping.objectTypeMap[iter->second] = ObjectDataLoader::getObjectTypeFromName(iter->first);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

bool GameSaveIO::buildPlanetDataVersionMapping(PlanetType planetType)
{
    createSaveDirectoryIfRequired();

    std::filesystem::path dir(getRootDir() + "Saves/" + fileName + "/version");
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }
    
    std::ofstream f(getPlanetDataVersionMappingFileName(planetType));
    
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

std::string GameSaveIO::getPlanetDataVersionMappingFileName(PlanetType planetType)
{
    std::string planetName = PlanetGenDataLoader::getPlanetGenData(planetType).name;
    return (getRootDir() + "Saves/" + fileName + "/version/" + planetName + "VersionMapping" + ".ver");
}

std::string GameSaveIO::getRootDir()
{
    return (sago::getDataHome() + "/Planeturem/");
}