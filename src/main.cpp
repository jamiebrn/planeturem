#include <SFML/Graphics.hpp>
#include <FastNoiseLite.h>
#include <math.h>
#include <array>
#include <map>
#include <iostream>
#include <memory>

#include "TextureManager.hpp"
#include "Shaders.hpp"
#include "ChunkManager.hpp"
#include "Camera.hpp"
#include "Helper.hpp"
#include "Object/Player.hpp"

#include "GUI/InventoryGUI.hpp"

int main()
{
    srand(time(NULL));

    auto window = sf::RenderWindow{ { 1280, 720 }, "spacebuild" };
    window.setFramerateLimit(165);

    TextureManager::loadTextures(window);
    Shaders::loadShaders();
    TextDraw::loadFont("Oswald-Regular.ttf");

    sf::Vector2f selectPos(0, 0);

    Player player({500, 200});

    bool inventoryOpen = false;

    // sf::Shader shader;
    // shader.loadFromFile("shader.frag", sf::Shader::Fragment);

    FastNoiseLite noise(rand());
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    sf::Clock clock;
    float time = 0;

    while (window.isOpen())
    {
        sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

        sf::Vector2i selectPosTile(std::floor((mousePos.x - Camera::getDrawOffset().x) / 48.0f), std::floor((mousePos.y - Camera::getDrawOffset().y) / 48.0f));

        float dt = clock.restart().asSeconds();
        time += dt;

        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::E)
                    inventoryOpen = !inventoryOpen;
            }

            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    if (!inventoryOpen)
                    {
                        WorldObject* selectedObject = ChunkManager::getSelectedObject(selectPosTile);
                        if (selectedObject)
                            selectedObject->interact();
                    }
                    else
                    {

                    }
                }
            }
        }

        sf::Vector2f mouseWorldPos = mousePos - Camera::getDrawOffset();
        selectPos.x = Helper::lerp(selectPos.x, std::floor(mouseWorldPos.x / 48) * 48, 25 * dt);
        selectPos.y = Helper::lerp(selectPos.y, std::floor(mouseWorldPos.y / 48) * 48, 25 * dt);

        Camera::update(player.getPosition(), dt);

        player.update(dt);

        ChunkManager::updateChunks(noise);
        ChunkManager::updateChunksObjects(dt);

        window.clear({80, 80, 80});

        ChunkManager::drawChunkTerrain(window);

        std::vector<WorldObject*> objects = ChunkManager::getChunkObjects();
        objects.push_back(&player);

        std::sort(objects.begin(), objects.end(), [](WorldObject* a, WorldObject* b)
        {
            return a->getPosition().y < b->getPosition().y;
        });

        for (WorldObject* object : objects)
        {
            object->draw(window);
        }

        if (!inventoryOpen)
        {
            TextureManager::drawTexture(window, {TextureType::SelectTile, selectPos + Camera::getIntegerDrawOffset(), 0, 3});
        }

        if (inventoryOpen)
        {
            InventoryGUI::draw(window);
        }

        window.display();

        window.setTitle("spacebuild - " + std::to_string((int)(1.0f / dt)) + "FPS");

    }
}
