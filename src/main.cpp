#include <SFML/Graphics.hpp>
#include <World/FastNoiseLite.h>
#include <Core/json.hpp>
#include <math.h>
#include <array>
#include <map>
#include <iostream>
#include <memory>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Helper.hpp"
#include "World/ChunkManager.hpp"
#include "Player/Player.hpp"
#include "Player/Cursor.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/BuildRecipeLoader.hpp"

#include "GUI/InventoryGUI.hpp"
#include "GUI/BuildGUI.hpp"

int main()
{
    srand(time(NULL));

    sf::VideoMode videoMode = sf::VideoMode::getDesktopMode();
    auto window = sf::RenderWindow{{videoMode.width, videoMode.height}, "spacebuild"};
    window.setFramerateLimit(165);
    window.setVerticalSyncEnabled(true);
    // window.setMouseCursorVisible(false);

    TextureManager::loadTextures(window);
    Shaders::loadShaders();
    TextDraw::loadFont("Data/upheavtt.ttf");

    ItemDataLoader::loadData("Data/Info/item_data.data");
    ObjectDataLoader::loadData("Data/Info/object_data.data");
    BuildRecipeLoader::loadData("Data/Info/build_recipes.data");

    ResolutionHandler::setResolution({videoMode.width, videoMode.height});

    // sf::Vector2f selectPos(0, 0);

    sf::View view({videoMode.width / 2.0f, videoMode.height / 2.0f}, {(float)videoMode.width, (float)videoMode.height});

    Player player({500, 200});

    bool inventoryOpen = false;
    bool buildMenuOpen = false;

    FastNoiseLite noise(rand());
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    sf::Clock clock;
    float time = 0;

    while (window.isOpen())
    {
        // sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

        // sf::Vector2i selectPosTile(std::floor((mousePos.x - Camera::getDrawOffset().x) / 48.0f), std::floor((mousePos.y - Camera::getDrawOffset().y) / 48.0f));
        // sf::Vector2f selectSize(1, 1);

        // ChunkPosition selected_chunk(std::floor(selectPosTile.x / 8.0f), std::floor(selectPosTile.y / 8.0f));
        // sf::Vector2i selected_tile(((selectPosTile.x % 8) + 8) % 8, ((selectPosTile.y % 8) + 8) % 8);

        float dt = clock.restart().asSeconds();
        time += dt;

        BuildableObject* selectedObject = ChunkManager::getSelectedObject(Cursor::getSelectedChunk(), Cursor::getSelectedChunkTile());

        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::Resized)
            {
                // std::cout << "s" << std::endl;
                view.setSize(event.size.width, event.size.height);
                view.setCenter({event.size.width / 2.0f, event.size.height / 2.0f});
                ResolutionHandler::setResolution({event.size.width, event.size.height});
            }

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::F11)
                {
                    sf::VideoMode videoMode = sf::VideoMode::getDesktopMode();
                    window.create(videoMode, "spacebuild", sf::Style::Default);
                    view.setSize({(float)videoMode.width, (float)videoMode.height});
                    view.setCenter({videoMode.width / 2.0f, videoMode.height / 2.0f});
                    ResolutionHandler::setResolution({videoMode.width, videoMode.height});
                }

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
                        if (Inventory::canBuildObject(objectType) && ChunkManager::canPlaceObject(Cursor::getSelectedChunk(), Cursor::getSelectedChunkTile(), objectType))
                        {
                            // Take resources
                            for (auto& itemPair : BuildRecipeLoader::getBuildRecipe(objectType).itemRequirements)
                            {
                                Inventory::takeItem(itemPair.first, itemPair.second);
                            }

                            // Build object
                            ChunkManager::setObject(Cursor::getSelectedChunk(), Cursor::getSelectedChunkTile(), objectType);
                        }
                    }
                }
            }

            if (event.type == sf::Event::MouseWheelScrolled)
            {
                BuildGUI::changeSelectedObject(-event.mouseWheelScroll.delta);
            }
        }

        // sf::Vector2f mouseWorldPos = mousePos - Camera::getDrawOffset();

        // if (selectedObject && !buildMenuOpen)
        // {
        //     selectSize = static_cast<sf::Vector2f>(ObjectDataLoader::getObjectData(selectedObject->getObjectType()).size);
        //     selectPos.x = Helper::lerp(selectPos.x, selectedObject->getPosition().x - 24.0f, 25 * dt);
        //     selectPos.y = Helper::lerp(selectPos.y, selectedObject->getPosition().y - 24.0f, 25 * dt);
        // }
        // else
        // {
        //     selectPos.x = Helper::lerp(selectPos.x, std::floor(mouseWorldPos.x / 48) * 48, 25 * dt);
        //     selectPos.y = Helper::lerp(selectPos.y, std::floor(mouseWorldPos.y / 48) * 48, 25 * dt);
        // }

        // if (buildMenuOpen)
        // {
        //     selectSize = static_cast<sf::Vector2f>(ObjectDataLoader::getObjectData(BuildGUI::getSelectedObject()).size);
        // }

        Camera::update(player.getPosition(), dt);
        Cursor::updateTileCursor(window, dt, buildMenuOpen);

        player.update(dt);

        ChunkManager::updateChunks(noise);
        ChunkManager::updateChunksObjects(dt);

        window.clear({80, 80, 80});

        window.setView(view);

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
            // TextureManager::drawSubTexture(window, {
            //     TextureType::SelectTile, selectPos + Camera::getIntegerDrawOffset(), 0, 3}, sf::IntRect(0, 0, 16, 16)); // top left
            // TextureManager::drawSubTexture(window, {
            //     TextureType::SelectTile, selectPos + sf::Vector2f(selectSize.x - 1, 0) * 48.0f + Camera::getIntegerDrawOffset(), 0, 3},
            //     sf::IntRect(16, 0, 16, 16)); // top right
            // TextureManager::drawSubTexture(window, {
            //     TextureType::SelectTile, selectPos + sf::Vector2f(0, selectSize.y - 1) * 48.0f + Camera::getIntegerDrawOffset(), 0, 3},
            //     sf::IntRect(32, 0, 16, 16)); // bottom left
            // TextureManager::drawSubTexture(window, {
            //     TextureType::SelectTile, selectPos + sf::Vector2f(selectSize.x - 1, selectSize.y - 1) * 48.0f + Camera::getIntegerDrawOffset(), 0, 3},
            //     sf::IntRect(48, 0, 16, 16)); // bottom right
            Cursor::drawTileCursor(window);
        }

        if (inventoryOpen)
        {
            InventoryGUI::draw(window);
        }

        if (buildMenuOpen)
        {
            BuildGUI::draw(window);

            unsigned int selectedObjectType = BuildGUI::getSelectedObject();

            BuildableObject recipeObject(Cursor::getLerpedSelectPos() + sf::Vector2f(24, 24), selectedObjectType);

            if (Inventory::canBuildObject(selectedObjectType) && ChunkManager::canPlaceObject(Cursor::getSelectedChunk(), Cursor::getSelectedChunkTile(), selectedObjectType))
                recipeObject.draw(window, dt, {0, 255, 0, 180});
            else
                recipeObject.draw(window, dt, {255, 0, 0, 180});
        }

        window.display();

        window.setTitle("spacebuild - " + std::to_string((int)(1.0f / dt)) + "FPS");

    }
}
