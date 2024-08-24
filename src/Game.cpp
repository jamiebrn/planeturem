#include "Game.hpp"

// FIX: Horse in the water? - also saw horse teleporting on collision with water (may be related)
// FIX: Cliffs are broken again? (cliff on grass field) - maybe fixed????

// PRIORITY: HIGH
// TODO: Make hotbar from inventory
// TODO: Allow changing of tools through hotbar selection

// PRIORITY: LOW
// TODO: Inventory item added notifications (maybe taking items?). Add in player class

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
    if(!TextDraw::loadFont("Data/Fonts/upheavtt.ttf")) return false;
    if(!Sounds::loadSounds()) return false;

    if(!ObjectDataLoader::loadData("Data/Info/object_data.data")) return false;
    if(!ItemDataLoader::loadData("Data/Info/item_data.data")) return false;
    if(!RecipeDataLoader::loadData("Data/Info/item_recipes.data")) return false;
    // if(!BuildRecipeLoader::loadData("Data/Info/build_recipes.data")) return false;
    if(!EntityDataLoader::loadData("Data/Info/entity_data.data")) return false;
    if(!ToolDataLoader::loadData("Data/Info/tool_data.data")) return false;

    // Load icon
    if(!icon.loadFromFile("Data/Textures/icon.png")) return false;
    window.setIcon(256, 256, icon.getPixelsPtr());

    // Load Steam API
    steamInitialised = SteamAPI_Init();
    if (steamInitialised)
        SteamUserStats()->RequestCurrentStats();

    // Randomise
    srand(time(NULL));

    // Create noise
    noise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
    noise.SetSeed(rand());
    noise.SetFrequency(0.1);

    // Initialise values
    gameTime = 0;
    gameState = GameState::OnPlanet;
    worldMenuState = WorldMenuState::Main;
    interactedObjectID = 0;
    interactedObjectPos = sf::Vector2f(0, 0);

    // Set world size
    worldSize = 240;

    // Initialise day/night cycle
    dayNightToggleTimer = 0.0f;
    worldDarkness = 0.0f;
    isDay = true;

    generateWaterNoiseTexture();

    // Find valid player spawn
    sf::Vector2f spawnPos = chunkManager.findValidSpawnPosition(2, noise, worldSize);
    player.setPosition(spawnPos);

    Camera::instantUpdate(player.getPosition());

    Sounds::playMusic(MusicType::Main);

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

    // float beforeScale = ResolutionHandler::getScale();

    ResolutionHandler::setResolution({newSize.x, newSize.y});

    Camera::instantUpdate(player.getPosition());

    // float afterScale = ResolutionHandler::getScale();

    // if (beforeScale != afterScale)
        // Camera::handleScaleChange(beforeScale, afterScale, player.getPosition());
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
        float dt = clock.restart().asSeconds();
        gameTime += dt;

        SteamAPI_RunCallbacks();

        window.setView(view);

        switch (gameState)
        {
            case GameState::Menu:
                runMenu(dt);
                break;
            case GameState::InShip:
                runInShip(dt);
                break;
            case GameState::OnPlanet:
                runOnPlanet(dt);
                break;
        }
    }
}

void Game::runMenu(float dt)
{
    for (auto event = sf::Event{}; window.pollEvent(event);)
    {
        handleEventsWindow(event);

        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Enter)
            {
                gameState = GameState::InShip;
            }
        }
    }

    window.clear();

    sf::Vector2f titlePos;
    titlePos.x = ResolutionHandler::getResolution().x / 2.0f;
    titlePos.y = 100.0f * ResolutionHandler::getResolutionIntegerScale();

    TextDraw::drawText(window, {
        "spacebuild", titlePos, {255, 255, 255}, static_cast<unsigned int>(48 * ResolutionHandler::getResolutionIntegerScale()), {0, 0, 0}, 0, true, false
        });

    sf::Vector2f continueTextPos;
    continueTextPos.x = titlePos.x;
    continueTextPos.y = ResolutionHandler::getResolution().y - 300.0f * ResolutionHandler::getResolutionIntegerScale();

    TextDraw::drawText(window, {
        "Enter to start", continueTextPos, {255, 255, 255}, static_cast<unsigned int>(30 * ResolutionHandler::getResolutionIntegerScale()), {0, 0, 0}, 0, true, false
        });

    window.display();
}

void Game::runInShip(float dt)
{
    for (auto event = sf::Event{}; window.pollEvent(event);)
    {
        handleEventsWindow(event);

        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Enter)
            {
                gameState = GameState::OnPlanet;
            }
        }
    }

    window.clear();

    sf::Vector2f textPos;
    textPos.x = ResolutionHandler::getResolution().x / 2.0f;
    textPos.y = ResolutionHandler::getResolution().y / 2.0f;

    TextDraw::drawText(window, {
        "In the ship", textPos, {255, 255, 255}, static_cast<unsigned int>(48 * ResolutionHandler::getResolutionIntegerScale()), {0, 0, 0}, 0, true, false
        });

    window.display();
}

void Game::runOnPlanet(float dt)
{
    sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    // Handle events
    for (auto event = sf::Event{}; window.pollEvent(event);)
    {
        handleEventsWindow(event);

        if (event.type == sf::Event::KeyPressed)
        {
            if (worldMenuState == WorldMenuState::Main)
            {
                if (event.key.code == sf::Keyboard::E)
                    worldMenuState = WorldMenuState::Inventory;
            }
            else
            {
                if (event.key.code == sf::Keyboard::E && worldMenuState == WorldMenuState::Inventory)
                {
                    InventoryGUI::handleClose();
                    worldMenuState = WorldMenuState::Main;
                }

                if (event.key.code == sf::Keyboard::Escape)
                {
                    InventoryGUI::handleClose();
                    worldMenuState = WorldMenuState::Main;
                }
            }
        }

        if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                switch (worldMenuState)
                {
                    case WorldMenuState::Main:
                        attemptUseTool();
                        break;
                    case WorldMenuState::Inventory:
                        if (!InventoryGUI::isMouseOverUI(mouseScreenPos))
                        {
                            attemptUseTool();
                            attemptBuildObject();
                        }
                        InventoryGUI::handleLeftClick(mouseScreenPos);
                        break;
                }
            }
            else if (event.mouseButton.button == sf::Mouse::Right)
            {
                switch (worldMenuState)
                {
                    case WorldMenuState::Main:
                        attemptObjectInteract();
                        break;
                    case WorldMenuState::Inventory:
                        InventoryGUI::handleRightClick(mouseScreenPos);
                        break;
                }
            }
        }

        if (event.type == sf::Event::MouseWheelScrolled)
        {
            switch (worldMenuState)
            {
                case WorldMenuState::Inventory:
                    if (!InventoryGUI::handleScroll(mouseScreenPos, -event.mouseWheelScroll.delta))
                        handleZoom(event.mouseWheelScroll.delta);
                    break;
                case WorldMenuState::Main:
                    handleZoom(event.mouseWheelScroll.delta);
                    break;
            }
        }
    }


    //
    // -- UPDATING --
    //

    // Update tweens
    floatTween.update(dt);

    // Update day / night cycle
    dayNightToggleTimer += dt;
    // if (dayNightToggleTimer >= 20.0f)
    // {
    //     dayNightToggleTimer = 0.0f;
    //     if (isDay) floatTween.startTween(&worldDarkness, 0.0f, 0.95f, 7, TweenTransition::Sine, TweenEasing::EaseInOut);
    //     else floatTween.startTween(&worldDarkness, 0.95f, 0.0f, 7, TweenTransition::Sine, TweenEasing::EaseInOut);
    //     isDay = !isDay;
    // }

    // Update camera
    Camera::update(player.getPosition(), mouseScreenPos, dt);

    // Update cursor
    Cursor::updateTileCursor(window, dt, worldSize, chunkManager, player.getCollisionRect(), InventoryGUI::getHeldObjectType());

    // Update player
    bool wrappedAroundWorld = false;
    sf::Vector2f wrapPositionDelta;
    player.update(dt, Cursor::getMouseWorldPos(window), chunkManager, worldSize, wrappedAroundWorld, wrapPositionDelta);

    // Handle world wrapping for camera and cursor, if player wrapped around
    if (wrappedAroundWorld)
    {
        Camera::handleWorldWrap(wrapPositionDelta);
        Cursor::handleWorldWrap(wrapPositionDelta);
        chunkManager.reloadChunks();
    }

    // Enable / disable cursor drawing depending on player reach
    if (InventoryGUI::getHeldObjectType() < 0)
        Cursor::setCanReachTile(player.canReachPosition(Cursor::getMouseWorldPos(window)));
    
    // Get nearby crafting stations
    nearbyCraftingStationLevels = chunkManager.getNearbyCraftingStationLevels(player.getChunkInside(worldSize), player.getChunkTileInside(), 4, worldSize);

    // Update inventory GUI available recipes if required
    if (worldMenuState == WorldMenuState::Inventory)
        InventoryGUI::updateAvailableRecipes(nearbyCraftingStationLevels);

    // Update (loaded) chunks
    chunkManager.updateChunks(noise, worldSize);
    chunkManager.updateChunksObjects(dt);
    chunkManager.updateChunksEntities(dt, worldSize);


    //
    // -- DRAWING --
    //

    window.clear({80, 80, 80});

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

    // UI

    switch (worldMenuState)
    {
        case WorldMenuState::Main:
            Cursor::drawCursor(window);
            break;
        
        case WorldMenuState::Inventory:
        {
            Cursor::drawCursor(window);
            
            ObjectType placeObject = InventoryGUI::getHeldObjectType();
            if (placeObject >= 0)
            {
                // Draw object to be placed if held
                bool canPlace = chunkManager.canPlaceObject(Cursor::getSelectedChunk(worldSize),
                                                            Cursor::getSelectedChunkTile(),
                                                            placeObject,
                                                            worldSize,
                                                            player.getCollisionRect());

                sf::Color drawColor(255, 0, 0, 180);
                if (canPlace)
                    drawColor = sf::Color(0, 255, 0, 180);
                
                BuildableObject objectGhost(Cursor::getLerpedSelectPos() + sf::Vector2f(TILE_SIZE_PIXELS_UNSCALED / 2.0f, TILE_SIZE_PIXELS_UNSCALED / 2.0f), placeObject);

                objectGhost.draw(window, dt, drawColor);
            }

            InventoryGUI::draw(window, mouseScreenPos);
            break;
        }
    }

    // Debug info
    {
        float intScale = static_cast<float>(ResolutionHandler::getResolutionIntegerScale());

        std::vector<std::string> debugStrings = {
            GAME_VERSION,
            std::to_string(static_cast<int>(1.0f / dt)) + "FPS",
            std::to_string(chunkManager.getLoadedChunkCount()) + " Chunks loaded",
            std::to_string(chunkManager.getGeneratedChunkCount()) + " Chunks generated",
            "Player pos: " + std::to_string(static_cast<int>(player.getPosition().x)) + ", " + std::to_string(static_cast<int>(player.getPosition().y))
        };

/*
        for (const auto& craftingStationLevel : nearbyCraftingStationLevels)
        {
            debugStrings.push_back(craftingStationLevel.first + " level " + std::to_string(craftingStationLevel.second));
        }

        for (int recipeIdx : InventoryGUI::getAvailableRecipes())
        {
            ItemType itemType = RecipeDataLoader::getRecipeData()[recipeIdx].product;
            const std::string& itemName = ItemDataLoader::getItemData(itemType).name;

            debugStrings.push_back("can craft " + itemName);
        }
*/

        int yPos = ResolutionHandler::getResolution().y - debugStrings.size() * 20 * intScale - 10 * intScale;
        int yIncrement = 20 * intScale;

        for (const std::string& string : debugStrings)
        {
            TextDraw::drawText(window, {string, sf::Vector2f(10 * intScale, yPos), sf::Color(255, 255, 255), static_cast<unsigned int>(20 * intScale)});
            yPos += yIncrement;
        }
    }

    window.display();

    // window.setTitle("spacebuild - " + std::to_string((int)(1.0f / dt)) + "FPS");
}

void Game::handleEventsWindow(sf::Event& event)
{
    if (event.type == sf::Event::Closed)
    {
        window.close();
        return;
    }

    if (event.type == sf::Event::Resized)
    {
        handleWindowResize(sf::Vector2u(event.size.width, event.size.height));
        return;
    }

    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::F11)
        {
            toggleFullScreen();
            return;
        }
    }
}

void Game::attemptUseTool()
{
    if (player.isUsingTool() || InventoryGUI::getHeldObjectType() >= 0)
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
        bool canDestroyObject = chunkManager.canDestroyObject(Cursor::getSelectedChunk(worldSize),
                                                        Cursor::getSelectedChunkTile(),
                                                        worldSize,
                                                        player.getCollisionRect());

        if (!canDestroyObject)
            return;

        std::optional<BuildableObject>& selectedObjectOptional = chunkManager.getChunkObject(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile());
        if (selectedObjectOptional.has_value())
        {
            BuildableObject& selectedObject = selectedObjectOptional.value();
            selectedObject.damage(1);
        }
    }
}

void Game::attemptObjectInteract()
{
    if (worldMenuState != WorldMenuState::Main)
        return;
    
    // Get mouse position in screen space and world space
    sf::Vector2f mouseWorldPos = Cursor::getMouseWorldPos(window);

    if (!player.canReachPosition(mouseWorldPos))
        return;
    
    std::optional<BuildableObject>& selectedObjectOptional = chunkManager.getChunkObject(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile());
    if (selectedObjectOptional.has_value())
    {
        BuildableObject& selectedObject = selectedObjectOptional.value();

        ObjectInteractionEventData interactionEvent = selectedObject.interact();
        if (interactionEvent.interactionType == ObjectInteraction::OpenFurnace)
        {
            // worldMenuState = WorldMenuState::Furnace;
            interactedObjectID = interactionEvent.objectID;
        }
    }
}

void Game::attemptBuildObject()
{
    ObjectType objectType = InventoryGUI::getHeldObjectType();

    if (objectType < 0)
        return;

    // bool canAfford = Inventory::canBuildObject(objectType);
    bool canPlace = chunkManager.canPlaceObject(Cursor::getSelectedChunk(worldSize),
                                                Cursor::getSelectedChunkTile(),
                                                objectType,
                                                worldSize,
                                                player.getCollisionRect());

    if (canPlace)
    {
        // Remove object from being held
        InventoryGUI::placeHeldObject();

        // Build object
        chunkManager.setObject(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile(), objectType, worldSize);
    }
}