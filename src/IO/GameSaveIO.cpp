#include "IO/GameSaveIO.hpp"

GameSaveIO::GameSaveIO(std::string fileName)
{
    this->fileName = fileName;
}

bool GameSaveIO::load(GameSave& gameSave)
{
    std::filesystem::path dir("Saves/" + fileName + ".dat");

    if (!std::filesystem::exists(dir))
    {
        return false;
    }

    std::fstream in("Saves/" + fileName + ".dat", std::ios::in | std::ios::binary);

    if (!in)
    {
        return false;
    }

    cereal::BinaryInputArchive archive(in);

    archive(gameSave);

    return true;
}

bool GameSaveIO::write(const GameSave& gameSave)
{
    std::filesystem::path dir("Saves");
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }

    std::fstream out("Saves/" + fileName + ".dat", std::ios::out | std::ios::binary);

    if (!out)
    {
        return false;
    }

    cereal::BinaryOutputArchive archive(out);

    archive(gameSave);

    return true;
}