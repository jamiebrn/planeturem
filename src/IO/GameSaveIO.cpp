#include "IO/GameSaveIO.hpp"

GameSaveIO::GameSaveIO(std::string fileName)
{
    this->fileName = fileName;
}

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
        playerGameSave.playerData = json["player-data"];
        playerGameSave.networkPlayerDatas = json["network-player-datas"];
        playerGameSave.time = json["time"];
        playerGameSave.day = json["day"];

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

        std::fstream in(getRootDir() + "Saves/" + fileName + "/Planets/" + planetName + ".dat", std::ios::in | std::ios::binary);
        
        if (!in)
        {
            return false;
        }

        // Read and decompress
        CompressedData compressedData;
        {
            cereal::BinaryInputArchive archive(in);
            archive(compressedData);
        }

        std::vector<char> decompressedData = compressedData.decompress();

        // Deserialise decompressed data
        std::stringstream stream(std::string(decompressedData.begin(), decompressedData.end()));
        {
            cereal::BinaryInputArchive archive(stream);
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

bool GameSaveIO::loadRoomDestinationSave(RoomType roomDestinationType, RoomDestinationGameSave& roomDestinationGameSave)
{
    try
    {
        const std::string& roomDestinationName = StructureDataLoader::getRoomData(roomDestinationType).name;

        std::fstream in(getRootDir() + "Saves/" + fileName + "/Rooms/" + roomDestinationName + ".dat", std::ios::in | std::ios::binary);
        
        if (!in)
        {
            return false;
        }

        cereal::BinaryInputArchive archive(in);

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
        json["player-data"] = playerGameSave.playerData;
        json["network-player-datas"] = playerGameSave.networkPlayerDatas;
        json["time"] = playerGameSave.time;
        json["day"] = playerGameSave.day;
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

        std::fstream out(getRootDir() + "Saves/" + fileName + "/Planets/" + planetName + ".dat", std::ios::out | std::ios::binary);

        // if (!out || !createAndWriteGameDataVersionMapping(getPlanetGameDataVersionMappingFileName(planetType)))
        if (!out)
        {
            throw std::invalid_argument("Could not open planet \"" + planetName + "\" file for \"" + fileName + "\"");
        }

        // Serialise and compress
        std::stringstream outputStream;
        {
            cereal::BinaryOutputArchive archive(outputStream);
            archive(planetGameSave);
        }

        std::string outputStreamStr = outputStream.str();
        std::vector<char> serialisedData(outputStreamStr.begin(), outputStreamStr.end());
        
        CompressedData compressedData(serialisedData);

        cereal::BinaryOutputArchive archive(out);
        archive(compressedData);

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

        std::fstream out(getRootDir() + "Saves/" + fileName + "/Rooms/" + roomDestinationName + ".dat", std::ios::out | std::ios::binary);

        // if (!out || !createAndWriteGameDataVersionMapping(getRoomDestinationGameDataVersionMappingFileName(roomDestinationType)))
        if (!out)
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

bool GameSaveIO::attemptDeleteSave()
{
    try
    {
        std::error_code ec;
        std::filesystem::permissions(getRootDir() + "Saves/" + fileName + "/",
            std::filesystem::perms::owner_all,
            std::filesystem::perm_options::add,
            ec);
        if (ec) {
            std::cerr << "Failed to modify permissions: " << ec.message() << '\n';
        }
        std::filesystem::remove_all(getRootDir() + "Saves/" + fileName + "/", ec);
        if (ec) {
            std::cerr << ec.message() << '\n';
        }
        std::filesystem::remove(getRootDir() + "Saves/" + fileName + "/");
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return false;
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

        saveFileSummary.playerData = playerSave.playerData;

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

bool GameSaveIO::writeOptionsSave(const OptionsSave& optionsSave)
{
    std::fstream out(getRootDir() + "config.json", std::ios::out);

    if (!out)
    {
        return false;
    }

    try
    {
        nlohmann::json json;
        json["music-volume"] = optionsSave.musicVolume;
        json["sound-volume"] = optionsSave.soundVolume;
        json["screen-shake-enabled"] = optionsSave.screenShakeEnabled;
        json["controller-glyph-type"] = optionsSave.controllerGlyphType;
        json["v-sync"] = optionsSave.vSync;
        
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

bool GameSaveIO::loadOptionsSave(OptionsSave& optionsSave)
{
    std::fstream in(getRootDir() + "config.json", std::ios::in);

    if (!in)
    {
        return false;
    }

    try
    {
        nlohmann::json json = nlohmann::json::parse(in);
        optionsSave.musicVolume = json["music-volume"];

        if (json.contains("sound-volume"))
        {
            optionsSave.soundVolume = json.at("sound-volume");
        }

        if (json.contains("screen-shake-enabled"))
        {
            optionsSave.screenShakeEnabled = json.at("screen-shake-enabled");
        }

        if (json.contains("controller-glyph-type"))
        {
            optionsSave.controllerGlyphType = json.at("controller-glyph-type");
        }

        if (json.contains("v-sync"))
        {
            optionsSave.vSync = json.at("v-sync");
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;   
}

bool GameSaveIO::writeInputBindingsSave(const InputBindingsSave& inputBindingsSave)
{
    std::fstream out(getRootDir() + "input-bindings.json", std::ios::out);

    if (!out)
    {
        return false;
    }

    try
    {
        nlohmann::json json = inputBindingsSave;

        out << json.dump(1, '\t');
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

bool GameSaveIO::loadInputBindingsSave(InputBindingsSave& inputBindingsSave)
{
    std::fstream in(getRootDir() + "input-bindings.json", std::ios::in);

    if (!in)
    {
        return false;
    }

    try
    {
        nlohmann::json json = nlohmann::json::parse(in);

        inputBindingsSave = json;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;   
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

    dir = std::filesystem::path(getRootDir() + "Saves/" + fileName + "/Planets/");
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }

    dir = std::filesystem::path(getRootDir() + "Saves/" + fileName + "/Rooms/");
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }
}

std::string GameSaveIO::getRootDir()
{
    return (sago::getDataHome() + "/Planeturem/");
}