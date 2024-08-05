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
    if(!EntityDataLoader::loadData("Data/Info/entity_data.data")) return false;

    // Randomise
    srand(time(NULL));

    // Create noise
    noise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
    noise.SetSeed(rand());
    noise.SetFrequency(0.1);

    // Initialise values
    inventoryOpen = false;
    buildMenuOpen = false;
    gameTime = 0;

    // Set world size
    worldSize = 40;

    generateWaterNoiseTexture();

    // Return true by default
    return true;
}

void Game::generateWaterNoiseTexture()
{
    // Create noise generators for water texture
    FastNoise waterNoise(rand());
    FastNoise waterNoiseTwo(rand());

    // Initialise noise values
    waterNoise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
    waterNoise.SetFrequency(0.1);
    waterNoiseTwo.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
    waterNoiseTwo.SetFrequency(0.1);

    // Constant storing size of water texture
    static constexpr int waterNoiseSize = 16 * 8;

    // Create arrays to store sampled noise data
    std::array<std::array<sf::Uint8, waterNoiseSize * 4>, waterNoiseSize> noiseData;
    std::array<std::array<sf::Uint8, waterNoiseSize * 4>, waterNoiseSize> noiseTwoData;

    // Sample noise data
    for (int y = 0; y < noiseData.size(); y++)
    {
        for (int x = 0; x < noiseData[0].size() / 4; x++)
        {
            float noiseValue = waterNoise.GetNoiseSeamless2D(x, y, waterNoiseSize, waterNoiseSize);
            static constexpr float sqrt2Div2 = 0.70710678119f;
            float noiseValueNormalised = (noiseValue - sqrt2Div2) / (sqrt2Div2 * 2);
            noiseData[y][x * 4] = noiseValueNormalised * 255;
            noiseData[y][x * 4 + 1] = noiseValueNormalised * 255;
            noiseData[y][x * 4 + 2] = noiseValueNormalised * 255;
            noiseData[y][x * 4 + 3] = 255;

            noiseValue = waterNoiseTwo.GetNoiseSeamless2D(x, y, waterNoiseSize, waterNoiseSize);
            noiseValueNormalised = (noiseValue - sqrt2Div2) / (sqrt2Div2 * 2);
            noiseTwoData[y][x * 4] = noiseValueNormalised * 255;
            noiseTwoData[y][x * 4 + 1] = noiseValueNormalised * 255;
            noiseTwoData[y][x * 4 + 2] = noiseValueNormalised * 255;
            noiseTwoData[y][x * 4 + 3] = 255;
        }
    }

    // Load sampled data into images, then load into textures to pass into shader
    std::array<sf::Image, 2> waterNoiseImages;

    waterNoiseImages[0].create(waterNoiseSize, waterNoiseSize, noiseData.data()->data());
    waterNoiseImages[1].create(waterNoiseSize, waterNoiseSize, noiseTwoData.data()->data());
    
    waterNoiseTextures[0].loadFromImage(waterNoiseImages[0]);
    waterNoiseTextures[1].loadFromImage(waterNoiseImages[1]);

    // Pass noise textures into water shader
    sf::Shader* waterShader = Shaders::getShader(ShaderType::Water);
    waterShader->setUniform("noise", waterNoiseTextures[0]);
    waterShader->setUniform("noiseTwo", waterNoiseTextures[1]);

    // Set water color (blue)
    waterShader->setUniform("waterColor", sf::Glsl::Vec4(77 / 255.0f, 155 / 255.0f, 230 / 255.0f, 1.0f));
}

void Game::run()
{
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        gameTime += dt;

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
        chunkManager.updateChunksEntities(dt, worldSize);

        window.clear({80, 80, 80});

        window.setView(view);

        // Draw water
        chunkManager.drawChunkWater(window, gameTime);

        // Draw objects for reflection FUTURE
        std::vector<WorldObject*> worldObjects = chunkManager.getChunkObjects();
        std::vector<WorldObject*> entities = chunkManager.getChunkEntities();
        worldObjects.insert(worldObjects.end(), entities.begin(), entities.end());
        worldObjects.push_back(&player);

        std::sort(worldObjects.begin(), worldObjects.end(), [](WorldObject* a, WorldObject* b)
        {
            if (a->getDrawLayer() != b->getDrawLayer()) return a->getDrawLayer() > b->getDrawLayer();
            if (a->getPosition().y == b->getPosition().y) return a->getPosition().x < b->getPosition().x;
            return a->getPosition().y < b->getPosition().y;
        });

        // Draw terrain
        chunkManager.drawChunkTerrain(window, gameTime);

        // Draw cursor
        std::optional<BuildableObject>& selectedObjectOptional = chunkManager.getChunkObject(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile());
        if (!inventoryOpen && selectedObjectOptional.has_value() || buildMenuOpen)
        {
            Cursor::drawTileCursor(window);
        }

        // Draw objects
        for (WorldObject* worldObject : worldObjects)
        {
            worldObject->draw(window, dt, {255, 255, 255, 255});
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