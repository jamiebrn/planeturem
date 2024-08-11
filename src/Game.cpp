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
    if(!ToolDataLoader::loadData("Data/Info/tool_data.data")) return false;

    // Load icon
    if(!icon.loadFromFile("Data/icon.png")) return false;
    window.setIcon(256, 256, icon.getPixelsPtr());

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

    // Initialise day/night cycle
    dayNightToggleTimer = 0.0f;
    worldDarkness = 0.0f;
    isDay = true;

    generateWaterNoiseTexture();

    // Return true by default
    return true;
}

void Game::toggleFullScreen()
{
    fullScreen = !fullScreen;

    sf::VideoMode videoMode = sf::VideoMode::getDesktopMode();

    unsigned int windowStyle = sf::Style::Default;
    if (fullScreen) windowStyle = sf::Style::None;
    
    window.create(videoMode, "spacebuild", windowStyle);

    // Set window stuff
    window.setIcon(256, 256, icon.getPixelsPtr());
    window.setFramerateLimit(165);
    window.setVerticalSyncEnabled(true);

    handleWindowResize(sf::Vector2u(videoMode.width, videoMode.height));
}

void Game::handleWindowResize(sf::Vector2u newSize)
{
    view.setSize(newSize.x, newSize.y);
    view.setCenter({newSize.x / 2.0f, newSize.y / 2.0f});

    float beforeScale = ResolutionHandler::getScale();

    ResolutionHandler::setResolution({newSize.x, newSize.y});

    float afterScale = ResolutionHandler::getScale();

    if (beforeScale != afterScale)
        Camera::handleScaleChange(beforeScale, afterScale, player.getPosition());
}

void Game::handleZoom(int zoomChange)
{
    float beforeScale = ResolutionHandler::getScale();
    ResolutionHandler::changeZoom(zoomChange);
    
    float afterScale = ResolutionHandler::getScale();

    Camera::handleScaleChange(beforeScale, afterScale, player.getPosition());
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
            noiseValue = FastNoise::Normalise(noiseValue);
            noiseData[y][x * 4] = noiseValue * 255;
            noiseData[y][x * 4 + 1] = noiseValue * 255;
            noiseData[y][x * 4 + 2] = noiseValue * 255;
            noiseData[y][x * 4 + 3] = 255;

            noiseValue = waterNoiseTwo.GetNoiseSeamless2D(x, y, waterNoiseSize, waterNoiseSize);
            noiseValue = FastNoise::Normalise(noiseValue);
            noiseTwoData[y][x * 4] = noiseValue * 255;
            noiseTwoData[y][x * 4 + 1] = noiseValue * 255;
            noiseTwoData[y][x * 4 + 2] = noiseValue * 255;
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
        // std::cout << ResolutionHandler::getScale() << std::endl;
        
        float dt = clock.restart().asSeconds();
        gameTime += dt;


        floatTween.update(dt);

        handleEvents();        

        dayNightToggleTimer += dt;
        // if (dayNightToggleTimer >= 20.0f)
        // {
        //     dayNightToggleTimer = 0.0f;
        //     if (isDay) floatTween.startTween(&worldDarkness, 0.0f, 0.95f, 7, TweenTransition::Sine, TweenEasing::EaseInOut);
        //     else floatTween.startTween(&worldDarkness, 0.95f, 0.0f, 7, TweenTransition::Sine, TweenEasing::EaseInOut);
        //     isDay = !isDay;
        // }

        Camera::update(player.getPosition(), dt);
        Cursor::updateTileCursor(window, dt, buildMenuOpen, worldSize, chunkManager);

        player.update(dt, Cursor::getMouseWorldPos(window), chunkManager);

        if (!buildMenuOpen)
            Cursor::setCanReachTile(player.canReachPosition(Cursor::getMouseWorldPos(window)));

        chunkManager.updateChunks(noise, worldSize);
        chunkManager.updateChunksObjects(dt);
        chunkManager.updateChunksEntities(dt, worldSize);

        window.clear({80, 80, 80});

        window.setView(view);

        // Draw all world onto texture for lighting
        sf::RenderTexture worldTexture;
        worldTexture.create(window.getSize().x, window.getSize().y);
        worldTexture.clear();

        // Draw water
        chunkManager.drawChunkWater(worldTexture, gameTime);

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
        chunkManager.drawChunkTerrain(worldTexture, gameTime);


        // Draw objects
        for (WorldObject* worldObject : worldObjects)
        {
            worldObject->draw(worldTexture, dt, {255, 255, 255, 255});
        }

        worldTexture.display();

        // Draw light sources on light texture
        sf::RenderTexture lightTexture;
        lightTexture.create(window.getSize().x, window.getSize().y);
        lightTexture.clear({0, 0, 0, 0});

        player.drawLightMask(lightTexture);

        for (WorldObject* entity : entities)
        {
            Entity* entityCasted = static_cast<Entity*>(entity);
            entityCasted->drawLightMask(lightTexture);
        }

        lightTexture.display();

        // Finish drawing world - draw world texture
        sf::Sprite worldTextureSprite(worldTexture.getTexture());
        sf::Shader* lightingShader = Shaders::getShader(ShaderType::Lighting);
        lightingShader->setUniform("lightingTexture", lightTexture.getTexture());
        lightingShader->setUniform("darkness", worldDarkness);
        window.draw(worldTextureSprite, lightingShader);

        // Draw cursor

        if (!inventoryOpen)
        {
            Cursor::drawCursor(window);
        }

        if (inventoryOpen)
        {
            InventoryGUI::draw(window);
        }

        if (buildMenuOpen)
        {
            BuildGUI::draw(window);

            unsigned int selectedObjectType = BuildGUI::getSelectedObject();

            BuildableObject recipeObject(Cursor::getLerpedSelectPos() + sf::Vector2f(TILE_SIZE_PIXELS_UNSCALED / 2.0f, TILE_SIZE_PIXELS_UNSCALED / 2.0f), selectedObjectType);

            if (Inventory::canBuildObject(selectedObjectType) && chunkManager.canPlaceObject(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile(), selectedObjectType, worldSize))
                recipeObject.draw(window, dt, {0, 255, 0, 180});
            else
                recipeObject.draw(window, dt, {255, 0, 0, 180});
        }

        window.display();

        window.setTitle("spacebuild - " + std::to_string((int)(1.0f / dt)) + "FPS");

    }
}

void Game::handleEvents()
{
    for (auto event = sf::Event{}; window.pollEvent(event);)
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
        }

        if (event.type == sf::Event::Resized)
        {
            handleWindowResize(sf::Vector2u(event.size.width, event.size.height));
        }

        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::F11)
            {
                toggleFullScreen();
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
                attemptUseTool();
                attemptBuildObject();
            }
        }

        if (event.type == sf::Event::MouseWheelScrolled)
        {
            if (buildMenuOpen)
                BuildGUI::changeSelectedObject(-event.mouseWheelScroll.delta);
            else
                handleZoom(event.mouseWheelScroll.delta);
        }
    }
}

void Game::attemptUseTool()
{
    if (inventoryOpen || buildMenuOpen || player.isUsingTool())
        return;

    // Get mouse position in screen space and world space
    sf::Vector2f mouseWorldPos = Cursor::getMouseWorldPos(window);

    player.useTool();

    if (!player.canReachPosition(mouseWorldPos))
        return;

    Entity* selectedEntity = chunkManager.getSelectedEntity(Cursor::getSelectedChunk(worldSize), mouseWorldPos);
    if (selectedEntity != nullptr)
    {
        selectedEntity->damage(1);
    }
    else
    {
        std::optional<BuildableObject>& selectedObjectOptional = chunkManager.getChunkObject(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile());
        if (selectedObjectOptional.has_value())
        {
            BuildableObject& selectedObject = selectedObjectOptional.value();
            selectedObject.damage(1);
        }
    }
}

void Game::attemptBuildObject()
{
    if (!buildMenuOpen)
        return;
    
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