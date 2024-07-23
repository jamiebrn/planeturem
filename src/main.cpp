#include <SFML/Graphics.hpp>
#include <FastNoiseLite.h>
#include <math.h>
#include <array>
#include <map>
#include <iostream>
#include <memory>

#include "TextureManager.hpp"
#include "ChunkManager.hpp"
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

        ChunkManager::updateChunks(noise);

        window.clear({80, 80, 80});

        ChunkManager::drawChunkTerrain(window);

        for (auto& chunkPair : ChunkManager::getChunks())
        {
            ChunkPosition chunkPos = chunkPair.first;
            std::unique_ptr<Chunk>& chunk = chunkPair.second;
            
            chunk->drawChunkObjects(window);
        }

        TextureManager::drawTexture(window, {TextureType::SelectTile, selectPos + Camera::getIntegerDrawOffset(), 0, 3});

        TextureManager::drawTexture(window, {TextureType::Player, playerPos + Camera::getIntegerDrawOffset(), 0, 3});

        window.display();

        window.setTitle("spacebuild - " + std::to_string((int)(1.0f / dt)) + "FPS");

    }
}
