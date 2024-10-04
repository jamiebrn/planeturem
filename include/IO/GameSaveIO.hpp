#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <iostream>

#include <SFML/System/Vector2.hpp>

#include "World/ChunkPosition.hpp"
#include "World/Chunk.hpp"
#include "Player/InventoryData.hpp"

class GameSaveIO
{
public:
    GameSaveIO(std::string fileName);

    void beginWrite();
    void beginLoad();

    void end();

    int loadSeed();
    void writeSeed(int seed);

    sf::Vector2f loadPosition();
    void writePosition(sf::Vector2f position);

    InventoryData loadInventory();
    void writeInventory(InventoryData inventory);

    std::unordered_map<ChunkPosition, Chunk> loadChunks();
    void writeChunks(const std::unordered_map<ChunkPosition, Chunk>& chunks);

private:
    std::string fileName;

    std::fstream io;
};