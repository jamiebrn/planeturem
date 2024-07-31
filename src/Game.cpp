#include "Game.hpp"

Game::Game()
    : player(sf::Vector2f(0, 0)), window()
{}

bool Game::initialise()
{
    // Get screen resolution
    sf::VideoMode videoMode = sf::VideoMode::getDesktopMode();
    
    // Create window
    window.create(sf::VideoMode(videoMode.width, videoMode.height), "spacebuild", sf::Style::None);

    // Enable VSync and frame limit
    window.setFramerateLimit(165);
    window.setVerticalSyncEnabled(true);

    // Create game view from resolution
    view = sf::View({videoMode.width / 2.0f, videoMode.height / 2.0f}, {(float)videoMode.width, (float)videoMode.height});

    // Set resolution handler values
    ResolutionHandler::setResolution({videoMode.width, videoMode.height});

    // Load assets
    if(!TextureManager::loadTextures(window)) return false;
    if(!Shaders::loadShaders()) return false;
    if(!TextDraw::loadFont("Data/upheavtt.ttf")) return false;

    if(!ItemDataLoader::loadData("Data/Info/item_data.data")) return false;
    if(!ObjectDataLoader::loadData("Data/Info/object_data.data")) return false;
    if(!BuildRecipeLoader::loadData("Data/Info/build_recipes.data")) return false;

    // Randomise
    srand(time(NULL));

    // Create noise
    noise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
    noise.SetSeed(rand());
    noise.SetFrequency(0.1);

    // Initialise state values
    inventoryOpen = false;
    buildMenuOpen = false;

    // Return true by default
    return true;
}

void Game::run()
{
    sf::Clock clock;
    float time = 0;

    int worldSize = 40;

    // // Create noise texture for debugging
    // sf::VertexArray noiseVertexArray(sf::Quads, 8 * 8 * worldSize * worldSize * 4);
    // for (int y = 0; y < 8 * worldSize; y++)
    // {
    //     for (int x = 0; x < 8 * worldSize; x++)
    //     {
    //         // Sample noise
    //         float height = noise.GetNoiseSeamless2D(x, y, worldSize * 8, worldSize * 8);
    //         // float height = noise.GetNoise(x, y);
    //         // std::cout << height<< std::endl;

    //         int vertexArrayIndex = (x + y * 8 * worldSize) * 4;
    //         noiseVertexArray[vertexArrayIndex].position = sf::Vector2f(x * 2, y * 2);
    //         noiseVertexArray[vertexArrayIndex + 1].position = sf::Vector2f(x * 2 + 2, y * 2);
    //         noiseVertexArray[vertexArrayIndex + 3].position = sf::Vector2f(x * 2, y * 2 + 2);
    //         noiseVertexArray[vertexArrayIndex + 2].position = sf::Vector2f(x * 2 + 2, y * 2 + 2);

    //         float color = 255 / 2.0f + height * 255 / 2.0f;

    //         noiseVertexArray[vertexArrayIndex].color = {color, color, color};
    //         noiseVertexArray[vertexArrayIndex + 1].color = {color, color, color};
    //         noiseVertexArray[vertexArrayIndex + 3].color = {color, color, color};
    //         noiseVertexArray[vertexArrayIndex + 2].color = {color, color, color};
    //     }
    // }

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
                    window.setFramerateLimit(165);
                    window.setVerticalSyncEnabled(true);
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
                        std::optional<BuildableObject>& selectedObjectOptional = chunkManager.getChunkObject(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile());
                        if (selectedObjectOptional.has_value())
                        {
                            BuildableObject& selectedObject = selectedObjectOptional.value();
                            selectedObject.interact();
                        }
                    }
                    else if (buildMenuOpen)
                    {
                        unsigned int objectType = BuildGUI::getSelectedObject();
                        if (Inventory::canBuildObject(objectType) && chunkManager.canPlaceObject(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile(), objectType, worldSize))
                        {
                            // Take resources
                            for (auto& itemPair : BuildRecipeLoader::getBuildRecipe(objectType).itemRequirements)
                            {
                                Inventory::takeItem(itemPair.first, itemPair.second);
                            }

                            // Build object
                            chunkManager.setObject(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile(), objectType, worldSize);
                        }
                    }
                }
            }

            if (event.type == sf::Event::MouseWheelScrolled)
            {
                if (buildMenuOpen)
                    BuildGUI::changeSelectedObject(-event.mouseWheelScroll.delta);
            }
        }

        Camera::update(player.getPosition(), dt);
        Cursor::updateTileCursor(window, dt, buildMenuOpen, worldSize, chunkManager);

        player.update(dt, chunkManager);

        chunkManager.updateChunks(noise, worldSize);
        chunkManager.updateChunksObjects(dt);

        window.clear({80, 80, 80});

        window.setView(view);

        chunkManager.drawChunkTerrain(window);

        std::vector<WorldObject*> objects = chunkManager.getChunkObjects();
        objects.push_back(&player);

        std::sort(objects.begin(), objects.end(), [](WorldObject* a, WorldObject* b)
        {
            if (a->getDrawLayer() != b->getDrawLayer()) return a->getDrawLayer() > b->getDrawLayer();
            if (a->getPosition().y == b->getPosition().y) return a->getPosition().x < b->getPosition().x;
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

            float tileSize = ResolutionHandler::getTileSize();

            BuildableObject recipeObject(Cursor::getLerpedSelectPos() + sf::Vector2f(tileSize / 2.0f, tileSize / 2.0f), selectedObjectType);

            if (Inventory::canBuildObject(selectedObjectType) && chunkManager.canPlaceObject(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile(), selectedObjectType, worldSize))
                recipeObject.draw(window, dt, {0, 255, 0, 180});
            else
                recipeObject.draw(window, dt, {255, 0, 0, 180});
        }

        window.display();

        window.setTitle("spacebuild - " + std::to_string((int)(1.0f / dt)) + "FPS");

    }
}