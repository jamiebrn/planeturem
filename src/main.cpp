#include <SFML/Graphics.hpp>
#include <FastNoiseLite.h>
#include <math.h>
#include <array>
#include <map>
#include <iostream>
#include <memory>

#include "TextureManager.hpp"
#include "Chunk.hpp"

float lerp(float start, float dest, float weight)
{
    return start + weight * (dest - start);
}

struct ChunkPosition
{
    int x, y;
    ChunkPosition(int _x, int _y) : x(_x), y(_y) {}
    bool operator==(const ChunkPosition& other) const
    {
        return (x == other.x && y == other.y);
    }
    bool operator<(const ChunkPosition& other) const
    {
        if (y != other.y)
        {
            return y < other.y;
        }
        return x < other.x;
    }
};

int main()
{
    srand(time(NULL));

    auto window = sf::RenderWindow{ { 1280, 720 }, "spacebuild" };
    window.setFramerateLimit(165);

    TextureManager::loadTextures(window);

    sf::Vector2f playerPos(500, 200);
    sf::Vector2f cameraPos(0, 0);
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

        sf::Vector2f mouseWorldPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)) + cameraPos;
        selectPos.x = lerp(selectPos.x, std::floor(mouseWorldPos.x / 48) * 48, 25 * dt);
        selectPos.y = lerp(selectPos.y, std::floor(mouseWorldPos.y / 48) * 48, 25 * dt);

        cameraPos.x = lerp(cameraPos.x, playerPos.x - window.getSize().x / 2.0f, 6 * dt);
        cameraPos.y = lerp(cameraPos.y, playerPos.y - window.getSize().y / 2.0f, 6 * dt);

        // Chunk load/unload
        sf::Vector2f screenTopLeft = cameraPos;
        sf::Vector2f screenBottomRight = cameraPos + sf::Vector2f(1280, 720);
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

            chunk->drawChunk(window, cameraPos);
        }

        TextureManager::drawTexture(window, {TextureType::SelectTile, selectPos - cameraPos, 0, 3});

        TextureManager::drawTexture(window, {TextureType::Player, playerPos - cameraPos, 0, 3, {0.5, 0.5}});

        window.display();

        window.setTitle("spacebuild - " + std::to_string((int)(1.0f / dt)) + "FPS");

    }
}
