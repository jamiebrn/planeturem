#include "ChunkManager.hpp"

std::map<ChunkPosition, std::unique_ptr<Chunk>> ChunkManager::chunks;

void ChunkManager::updateChunks(const FastNoiseLite& noise)
{
    // Chunk load/unload
    sf::Vector2f screenTopLeft = -Camera::getDrawOffset();
    sf::Vector2f screenBottomRight = -Camera::getDrawOffset() + sf::Vector2f(1280, 720);
    sf::Vector2i screenTopLeftGrid(std::floor(screenTopLeft.x / (48 * 8)), std::floor(screenTopLeft.y / (48 * 8)));
    sf::Vector2i screenBottomRightGrid = screenTopLeftGrid + sf::Vector2i(std::ceil(1280.0f / (48 * 8)), std::ceil(720.0f / (48 * 8)));

    for (int y = screenTopLeftGrid.y; y <= screenBottomRightGrid.y; y++)
    {
        for (int x = screenTopLeftGrid.x; x <= screenBottomRightGrid.x; x++)
        {
            // Chunk already exists
            if (chunks.count(ChunkPosition(x, y)))
                continue;
            
            // Chunk does not exist, so generate a new chunk
            std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(sf::Vector2i(x, y));
            chunk->generateChunk(noise);

            chunks.emplace(ChunkPosition(x, y), std::move(chunk));
        }
    }

    for (auto& chunkPair : chunks)
    {
        ChunkPosition chunkPos = chunkPair.first;

        if (chunkPos.x < screenTopLeftGrid.x || chunkPos.x > screenBottomRightGrid.x
            || chunkPos.y < screenTopLeftGrid.y || chunkPos.y > screenBottomRightGrid.y)
        {
            // Unload chunk
            chunks.erase(chunkPos);
        }
    }
}

void ChunkManager::drawChunkTerrain(sf::RenderWindow& window)
{
    for (auto& chunkPair : chunks)
    {
        ChunkPosition chunkPos = chunkPair.first;
        std::unique_ptr<Chunk>& chunk = chunkPair.second;
        
        chunk->drawChunkTerrain(window);
    }
}