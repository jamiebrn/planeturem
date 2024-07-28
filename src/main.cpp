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
    auto window = sf::RenderWindow{{videoMode.width, videoMode.height}, "spacebuild", sf::Style::None};
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

    ChunkManager chunkManager;

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
        float dt = clock.restart().asSeconds();
        time += dt;

        BuildableObject* selectedObject = chunkManager.getChunkObject(Cursor::getSelectedChunk(), Cursor::getSelectedChunkTile());

        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::Resized)
            {
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
                        if (selectedObject)
                            selectedObject->interact();
                    }
                    else if (buildMenuOpen)
                    {
                        unsigned int objectType = BuildGUI::getSelectedObject();
                        if (Inventory::canBuildObject(objectType) && chunkManager.canPlaceObject(Cursor::getSelectedChunk(), Cursor::getSelectedChunkTile(), objectType))
                        {
                            // Take resources
                            for (auto& itemPair : BuildRecipeLoader::getBuildRecipe(objectType).itemRequirements)
                            {
                                Inventory::takeItem(itemPair.first, itemPair.second);
                            }

                            // Build object
                            chunkManager.setObject(Cursor::getSelectedChunk(), Cursor::getSelectedChunkTile(), objectType);
                        }
                    }
                }
            }

            if (event.type == sf::Event::MouseWheelScrolled)
            {
                BuildGUI::changeSelectedObject(-event.mouseWheelScroll.delta);
            }
        }

        Camera::update(player.getPosition(), dt);
        Cursor::updateTileCursor(window, dt, buildMenuOpen, chunkManager);

        player.update(dt, chunkManager);

        chunkManager.updateChunks(noise);
        chunkManager.updateChunksObjects(dt);

        window.clear({80, 80, 80});

        window.setView(view);

        chunkManager.drawChunkTerrain(window);

        std::vector<WorldObject*> objects = chunkManager.getChunkObjects();
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

            if (Inventory::canBuildObject(selectedObjectType) && chunkManager.canPlaceObject(Cursor::getSelectedChunk(), Cursor::getSelectedChunkTile(), selectedObjectType))
                recipeObject.draw(window, dt, {0, 255, 0, 180});
            else
                recipeObject.draw(window, dt, {255, 0, 0, 180});
        }

        window.display();

        window.setTitle("spacebuild - " + std::to_string((int)(1.0f / dt)) + "FPS");

    }
}
