#include "IO/GameSaveIO.hpp"

GameSaveIO::GameSaveIO(std::string fileName)
{
    this->fileName = fileName;
}

bool GameSaveIO::load(PlayerGameSave& playerGameSave, PlanetGameSave& planetGameSave)
{
    createSaveDirectoryIfRequired();

    std::filesystem::path dir("Saves/" + fileName + "/");

    if (!std::filesystem::exists(dir))
    {
        return false;
    }

    try
    {
        // Load player file
        {
            std::fstream in("Saves/" + fileName + "/Player.dat", std::ios::in | std::ios::binary);
            
            if (!in)
            {
                throw std::invalid_argument("Could not open player file for \"" + fileName + "\"");
            }

            cereal::BinaryInputArchive archive(in);

            archive(playerGameSave);
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

    std::fstream in("Saves/" + fileName + "/" + planetName + ".dat", std::ios::in | std::ios::binary);
    
    if (!in)
    {
        return false;
    }

    cereal::BinaryInputArchive archive(in);

    archive(planetGameSave);

    return true;
}

bool GameSaveIO::write(const PlayerGameSave& playerGameSave, const PlanetGameSave& planetGameSave)
{
    createSaveDirectoryIfRequired();

    std::filesystem::path dir("Saves/" + fileName + "/");
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }

    try
    {
        // Save player file
        {
            std::fstream out("Saves/" + fileName + "/Player.dat", std::ios::out | std::ios::binary);

            if (!out)
            {
                throw std::invalid_argument("Could not open player file for \"" + fileName + "\"");
            }

            cereal::BinaryOutputArchive archive(out);

            archive(playerGameSave);
        }

        // Save planet file
        {
            const std::string& planetName = PlanetGenDataLoader::getPlanetGenData(playerGameSave.planetType).name;

            std::fstream out("Saves/" + fileName + "/" + planetName + ".dat", std::ios::out | std::ios::binary);

            if (!out)
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

std::vector<std::string> GameSaveIO::getSaveFiles()
{
    createSaveDirectoryIfRequired();

    struct SaveFileWithDate
    {
        std::string name;
        std::filesystem::file_time_type lastModified;
    };

    std::vector<SaveFileWithDate> saveFilesWithDate;

    for (const auto& saveFile : std::filesystem::directory_iterator("Saves"))
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
    std::filesystem::path dir("Saves");
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }
}