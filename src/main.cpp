#include <SFML/Graphics.hpp>
#include <FastNoiseLite.h>
#include <math.h>
#include <array>
#include <map>
#include <iostream>
#include <memory>

#include "TextureManager.hpp"
#include "Chunk.hpp"
#include "ChunkPosition.hpp"
#include "Camera.hpp"
#include "Helper.hpp"

int main()
{
    srand(time(NULL));

    auto window = sf::RenderWindow{ { 1280, 720 }, "spacebuild" };
    window.setFramerateLimit(165);

    TextureManager::loadTextures(window);

    sf::Vector2f playerPos(500, 200);
    sf::Vector2f selectPos(0, 0);

    // sf::Shader shader;
    // shader.loadFromFile("shader.frag", sf::Shader::Fragment);

    FastNoiseLite noise(rand());
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    std::map<ChunkPosition, std::unique_ptr<Chunk>> chunks;

    sf::Clock clock;
    float time = 0;

    while (window.isOpen())
    {

        float dt = clock.restart().asSeconds();
        time += dt;

        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        sf::Vector2f direction;
        direction.x = sf::Keyboard::isKeyPressed(sf::Keyboard::D) - sf::Keyboard::isKeyPressed(sf::Keyboard::A);
        direction.y = sf::Keyboard::isKeyPressed(sf::Keyboard::S) - sf::Keyboard::isKeyPressed(sf::Keyboard::W);
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0)
            direction /= length;

        playerPos += direction * 300.0f * dt;

        sf::Vector2f mouseWorldPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)) - Camera::getDrawOffset();
        selectPos.x = Helper::lerp(selectPos.x, std::floor(mouseWorldPos.x / 48) * 48, 25 * dt);
        selectPos.y = Helper::lerp(selectPos.y, std::floor(mouseWorldPos.y / 48) * 48, 25 * dt);

        Camera::update(playerPos, dt);

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

        window.clear({80, 80, 80});

        for (auto& chunkPair : chunks)
        {
            ChunkPosition chunkPos = chunkPair.first;
            std::unique_ptr<Chunk>& chunk = chunkPair.second;
            
            chunk->drawChunkTerrain(window);
        }

        ChunkPosition playerChunkPos(std::floor(playerPos.x / (48 * 8)), std::floor(playerPos.y / (48 * 8)));

        for (auto& chunkPair : chunks)
        {
            ChunkPosition chunkPos = chunkPair.first;
            std::unique_ptr<Chunk>& chunk = chunkPair.second;

            if (chunkPos == playerChunkPos)
            {
                chunk->drawChunkObjects(window, &playerPos);
                continue;
            }
            
            chunk->drawChunkObjects(window);
        }

        TextureManager::drawTexture(window, {TextureType::SelectTile, selectPos + Camera::getDrawOffset(), 0, 3});

        window.display();

        window.setTitle("spacebuild - " + std::to_string((int)(1.0f / dt)) + "FPS");

    }
}
