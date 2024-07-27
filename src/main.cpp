#include <SFML/Graphics.hpp>
#include <World/FastNoiseLite.h>
#include <Core/json.hpp>
#include <math.h>
#include <array>
#include <map>
#include <iostream>
#include <memory>
#include <optional>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/Camera.hpp"
#include "Core/Helper.hpp"
#include "World/ChunkManager.hpp"
#include "Player/Player.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/BuildRecipeLoader.hpp"

#include "GUI/InventoryGUI.hpp"
#include "GUI/BuildGUI.hpp"

int main()
{
    srand(time(NULL));

    auto window = sf::RenderWindow{ { 1280, 720 }, "spacebuild" };
    window.setFramerateLimit(165);

    TextureManager::loadTextures(window);
    Shaders::loadShaders();
    TextDraw::loadFont("Data/upheavtt.ttf");

    ItemDataLoader::loadData("Data/Info/item_data.data");
    ObjectDataLoader::loadData("Data/Info/object_data.data");
    BuildRecipeLoader::loadData("Data/Info/build_recipes.data");

    sf::Vector2f selectPos(0, 0);

    Player player({500, 200});

    bool inventoryOpen = false;
    bool buildMenuOpen = false;

    FastNoiseLite noise(rand());
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    sf::Clock clock;
    float time = 0;

    while (window.isOpen())
    {
        sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

        sf::Vector2i selectPosTile(std::floor((mousePos.x - Camera::getDrawOffset().x) / 48.0f), std::floor((mousePos.y - Camera::getDrawOffset().y) / 48.0f));
        sf::Vector2f selectSize(1, 1);

        ChunkPosition selected_chunk(std::floor(selectPosTile.x / 8.0f), std::floor(selectPosTile.y / 8.0f));
        sf::Vector2i selected_tile(((selectPosTile.x % 8) + 8) % 8, ((selectPosTile.y % 8) + 8) % 8);

        BuildableObject* selectedObject = ChunkManager::getSelectedObject(selected_chunk, selected_tile);

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
                if (event.key.code == sf::Keyboard::E && !buildMenuOpen)
                    inventoryOpen = !inventoryOpen;
                
                if (event.key.code == sf::Keyboard::Tab && !inventoryOpen)
                    buildMenuOpen = !buildMenuOpen;
            }

            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    if (!inventoryOpen && !buildMenuOpen)
                    {
                        // ChunkPosition chunk(std::floor(selectPosTile.x / 8.0f), std::floor(selectPosTile.y / 8.0f));
                        // sf::Vector2i selected_tile(((selectPosTile.x % 8) + 8) % 8, ((selectPosTile.y % 8) + 8) % 8);

                        // BuildableObject* selectedObject = ChunkManager::getSelectedObject(chunk, selected_tile);
                        if (selectedObject)
                            selectedObject->interact();
                    }
                    else if (buildMenuOpen)
                    {
                        unsigned int objectType = BuildGUI::getSelectedObject();
                        if (Inventory::canBuildObject(objectType) && ChunkManager::canPlaceObject(selectPosTile))
                        {
                            // Take resources
                            for (auto& itemPair : BuildRecipeLoader::getBuildRecipe(objectType).itemRequirements)
                            {
                                Inventory::takeItem(itemPair.first, itemPair.second);
                            }

                            // Build object
                            ChunkManager::setObject(selectPosTile, objectType);
                        }
                    }
                }
            }

            if (event.type == sf::Event::MouseWheelScrolled)
            {
                BuildGUI::changeSelectedObject(-event.mouseWheelScroll.delta);
            }
        }

        sf::Vector2f mouseWorldPos = mousePos - Camera::getDrawOffset();

        if (selectedObject && !buildMenuOpen)
        {
            selectSize = static_cast<sf::Vector2f>(ObjectDataLoader::getObjectData(selectedObject->getObjectType()).size);
            selectPos.x = Helper::lerp(selectPos.x, selectedObject->getPosition().x - 24.0f, 25 * dt);
            selectPos.y = Helper::lerp(selectPos.y, selectedObject->getPosition().y - 24.0f, 25 * dt);
        }
        else
        {
            selectPos.x = Helper::lerp(selectPos.x, std::floor(mouseWorldPos.x / 48) * 48, 25 * dt);
            selectPos.y = Helper::lerp(selectPos.y, std::floor(mouseWorldPos.y / 48) * 48, 25 * dt);
        }

        if (buildMenuOpen)
        {
            selectSize = static_cast<sf::Vector2f>(ObjectDataLoader::getObjectData(BuildGUI::getSelectedObject()).size);
        }

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
            if (a->getDrawLayer() != b->getDrawLayer()) return a->getDrawLayer() > b->getDrawLayer();
            return a->getPosition().y < b->getPosition().y;
        });

        for (WorldObject* object : objects)
        {
            object->draw(window, dt, {255, 255, 255, 255});
        }

        if (!inventoryOpen)
        {
            // WHEN IMPLEMENTED IN SEPARATE CLASS
            // LERP EACH CORNER INDIVIDUALLY (looks bad at the moment)
            
            // Draw select tile
            TextureManager::drawSubTexture(window, {
                TextureType::SelectTile, selectPos + Camera::getIntegerDrawOffset(), 0, 3}, sf::IntRect(0, 0, 16, 16)); // top left
            TextureManager::drawSubTexture(window, {
                TextureType::SelectTile, selectPos + sf::Vector2f(selectSize.x - 1, 0) * 48.0f + Camera::getIntegerDrawOffset(), 0, 3},
                sf::IntRect(16, 0, 16, 16)); // top right
            TextureManager::drawSubTexture(window, {
                TextureType::SelectTile, selectPos + sf::Vector2f(0, selectSize.y - 1) * 48.0f + Camera::getIntegerDrawOffset(), 0, 3},
                sf::IntRect(32, 0, 16, 16)); // bottom left
            TextureManager::drawSubTexture(window, {
                TextureType::SelectTile, selectPos + sf::Vector2f(selectSize.x - 1, selectSize.y - 1) * 48.0f + Camera::getIntegerDrawOffset(), 0, 3},
                sf::IntRect(48, 0, 16, 16)); // bottom right
        }

        if (inventoryOpen)
        {
            InventoryGUI::draw(window);
        }

        if (buildMenuOpen)
        {
            BuildGUI::draw(window);

            unsigned int selectedObjectType = BuildGUI::getSelectedObject();

            BuildableObject recipeObject(selectPos + sf::Vector2f(24, 24), selectedObjectType);

            if (Inventory::canBuildObject(selectedObjectType) && ChunkManager::canPlaceObject(selectPosTile))
                recipeObject.draw(window, dt, {0, 255, 0, 180});
            else
                recipeObject.draw(window, dt, {255, 0, 0, 180});
        }

        window.display();

        window.setTitle("spacebuild - " + std::to_string((int)(1.0f / dt)) + "FPS");

    }
}
