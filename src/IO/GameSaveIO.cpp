#include "IO/GameSaveIO.hpp"

GameSaveIO::GameSaveIO(std::string fileName)
{
    this->fileName = fileName;
}

bool GameSaveIO::load(PlayerGameSave& playerGameSave, PlanetGameSave& planetGameSave)
{
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
        {
            const std::string& planetName = PlanetGenDataLoader::getPlanetGenData(playerGameSave.planetType).name;

            std::fstream in("Saves/" + fileName + "/" + planetName + ".dat", std::ios::in | std::ios::binary);
            
            if (!in)
            {
                throw std::invalid_argument("Could not open planet file for \"" + fileName + "\"");
            }

            cereal::BinaryInputArchive archive(in);

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

bool GameSaveIO::write(const PlayerGameSave& playerGameSave, const PlanetGameSave& planetGameSave)
{
    std::filesystem::path dir("Saves");
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }

    dir = std::filesystem::path("Saves/" + fileName + "/");
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