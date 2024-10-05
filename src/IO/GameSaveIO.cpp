#include "IO/GameSaveIO.hpp"

GameSaveIO::GameSaveIO(std::string fileName)
{
    this->fileName = fileName;
}

GameSave GameSaveIO::load()
{
    std::fstream in(fileName, std::ios::in | std::ios::binary);
    cereal::BinaryInputArchive archive(in);

    GameSave gameSave;
    archive(gameSave);

    return gameSave;
}

void GameSaveIO::write(GameSave gameSave)
{
    std::fstream out(fileName, std::ios::out | std::ios::binary);
    cereal::BinaryOutputArchive archive(out);

    archive(gameSave);
}

// void GameSaveIO::beginWrite()
// {
//     io = std::fstream(fileName, std::ios::out | std::ios::binary);
// }

// void GameSaveIO::beginLoad()
// {
//     io = std::fstream(fileName, std::ios::in | std::ios::binary);
// }

// void GameSaveIO::end()
// {
//     io.close();
// }

// int GameSaveIO::loadSeed()
// {
//     if (io)
//     {
//         int seed = 0;
//         io.read((char*)&seed, sizeof(seed));

//         return seed;
//     }
//     else
//     {
//         std::cout << "Error loading\n";
//     }

//     return 0;
// }

// void GameSaveIO::writeSeed(int seed)
// {
//     if (io)
//     {
//         io.write((char*)&seed, sizeof(seed));
//     }
//     else
//     {
//         std::cout << "Error saving\n";
//     }
// }

// sf::Vector2f GameSaveIO::loadPosition()
// {
//     sf::Vector2f pos;

//     io.read((char*)&pos, sizeof(pos));

//     return pos;
// }

// void GameSaveIO::writePosition(sf::Vector2f position)
// {
//     io.write((char*)&position, sizeof(position));
// }

// InventoryData GameSaveIO::loadInventory()
// {
//     InventoryData inventory;

//     inventory.readFromBinaryFile(io);

//     return inventory;
// }

// void GameSaveIO::writeInventory(InventoryData inventory)
// {
//     inventory.writeToBinaryFile(io);
// }

// std::unordered_map<ChunkPosition, Chunk> GameSaveIO::loadChunks()
// {
//     return {};
// }

// void GameSaveIO::writeChunks(const std::unordered_map<ChunkPosition, Chunk>& chunks)
// {
//     std::ofstream fout(fileName, std::ios::binary);


// }