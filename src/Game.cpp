#include "Game.hpp"

// FIX: Crash on save / rocket enter bug (can't save???)
// FIX: Collisions broken for objects when loading save (something to do with player death? or loading? maybe i was in god mode?)

// PRIORITY: HIGH
// TODO: Plants / farming / growing objects
// TODO: Create event callback system for object responding to triggers, rather than calling game functions directly

// TODO: Structure functionality (item spawns, crafting stations etc)
// TODO: Different types of structures
// TODO: Different types of tools? (fishing rod etc)

// PRIORITY: LOW
// TODO: Fishing rod draw order (split into separate objects?)


// -- Public methods / entry point -- //

Game::Game()
    : player(sf::Vector2f(0, 0), &armourInventory), window()
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

    // Hide mouse cursor
    window.setMouseCursorVisible(false);

    // Create game view from resolution
    view = sf::View({videoMode.width / 2.0f, videoMode.height / 2.0f}, {(float)videoMode.width, (float)videoMode.height});

    // Set resolution handler values
    ResolutionHandler::setResolution({videoMode.width, videoMode.height});

    // Load assets
    if(!TextureManager::loadTextures(window)) return false;
    if(!Shaders::loadShaders()) return false;
    if(!TextDraw::loadFont("Data/Fonts/upheavtt.ttf")) return false;
    if(!Sounds::loadSounds()) return false;

    // Load data
    if(!ItemDataLoader::loadData("Data/Info/items.data")) return false;
    if(!ObjectDataLoader::loadData("Data/Info/objects.data")) return false;
    if(!ToolDataLoader::loadData("Data/Info/tools.data")) return false;
    if(!ArmourDataLoader::loadData("Data/Info/armour.data")) return false;
    if(!EntityDataLoader::loadData("Data/Info/entities.data")) return false;
    if(!RecipeDataLoader::loadData("Data/Info/item_recipes.data")) return false;
    if(!StructureDataLoader::loadData("Data/Info/structures.data")) return false;
    if(!PlanetGenDataLoader::loadData("Data/Info/planet_generation.data")) return false;

    // Load icon
    if(!icon.loadFromFile("Data/Textures/icon.png")) return false;
    window.setIcon(256, 256, icon.getPixelsPtr());

    // Init ImGui
    if (!ImGui::SFML::Init(window)) return false;

    // Load Steam API
    steamInitialised = SteamAPI_Init();
    if (steamInitialised)
        SteamUserStats()->RequestCurrentStats();

    // Randomise
    srand(time(NULL));

    chunkManager.setWorldSize(240);
    // chunkManager.setSeed(20);
    // chunkManager.setPlanetType(PlanetGenDataLoader::getPlanetTypeFromName("Earthlike"));

    // Initialise values
    gameTime = 0;
    mainMenuState = MainMenuState::Main;
    gameState = GameState::MainMenu;
    destinationGameState = gameState;
    transitionGameStateTimer = 0.0f;
    worldMenuState = WorldMenuState::Main;

    menuScreenshotIndex = 0;
    menuScreenshotTimer = 0.0f;

    openedChestID = 0xFFFF;

    musicGapTimer = 0.0f;
    musicGap = 0.0f;

    inventory = InventoryData(32);
    armourInventory = InventoryData(3);

    // Initialise day/night cycle
    dayNightToggleTimer = 0.0f;
    worldDarkness = 0.0f;
    isDay = true;

    // Initialise GUI
    InventoryGUI::initialise(inventory);

    generateWaterNoiseTexture();

    // Find valid player spawn
    // sf::Vector2f spawnPos = chunkManager.findValidSpawnPosition(2);
    // player.setPosition(spawnPos);

    // Initialise inventory
    giveStartingInventory();

    Camera::instantUpdate(player.getPosition());

    // Return true by default
    return true;
}

void Game::run()
{
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        gameTime += dt;

        SteamAPI_RunCallbacks();
        ImGui::SFML::Update(window, sf::seconds(dt));

        Sounds::update(dt);

        window.setView(view);

        // runLightingTest();
        switch (gameState)
        {
            case GameState::MainMenu:
                runMainMenu(dt);
                break;
            
            case GameState::InStructure:
            case GameState::OnPlanet:
                runOnPlanet(dt);
                break;
        }

        if (isStateTransitioning())
        {
            updateStateTransition(dt);
            drawStateTransition();
        }

        drawMouseCursor();

        drawDebugMenu(dt);

        ImGui::SetMouseCursor(ImGui::GetIO().WantCaptureMouse ? ImGuiMouseCursor_Arrow : ImGuiMouseCursor_None);

        ImGui::SFML::Render(window);

        window.display();
    }
}


void Game::runLightingTest()
{
    for (auto event = sf::Event{}; window.pollEvent(event);)
    {
        handleEventsWindow(event);

        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::R)
            {
                lightingEngine.resetLightSources();
                lightingEngine.resetObstacles();
            }
        }
    }

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    int mouseTileX = mousePos.x / 12;
    int mouseTileY = mousePos.y / 12;

    lightingEngine.resetLighting();

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        lightingEngine.addLightSource(mouseTileX, mouseTileY, 0.7f);
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
    {
        lightingEngine.addObstacle(mouseTileX, mouseTileY, 0.7f);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        lightingEngine.addMovingLightSource(mouseTileX, mouseTileY, 0.7f);
    }

    lightingEngine.calculateLighting();

    window.clear();

    lightingEngine.drawObstacles(window);
    // lightingEngine.drawLighting(window, );
}


// -- Main Menu -- //

void Game::runMainMenu(float dt)
{
    for (auto event = sf::Event{}; window.pollEvent(event);)
    {
        handleEventsWindow(event);

        guiContext.processEvent(event);
    }

    // Drawing
    window.clear();

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    // Draw background
    static constexpr std::array<TextureType, 3> menuScreenshots = {TextureType::MenuScreenshot0, TextureType::MenuScreenshot1, TextureType::MenuScreenshot2};

    float menuScreenshotAlpha = 255.0f;

    menuScreenshotTimer += dt;
    if (menuScreenshotTimer >= 10.0f)
    {
        // Draw next screen shot behind and decrease alpha
        TextureManager::drawTexture(window, {.type = menuScreenshots[(menuScreenshotIndex + 1) % menuScreenshots.size()], .scale = sf::Vector2f(intScale, intScale)});

        menuScreenshotAlpha *= (2.0f - (menuScreenshotTimer - 10.0f)) / 2.0f;

        if (menuScreenshotTimer >= 12.0f)
        {
            menuScreenshotTimer = 0.0f;
            menuScreenshotIndex = (menuScreenshotIndex + 1) % menuScreenshots.size();
        }
    }

    TextureManager::drawTexture(window, {.type = menuScreenshots[menuScreenshotIndex], .scale = sf::Vector2f(intScale, intScale),
        .colour = sf::Color(255, 255, 255, menuScreenshotAlpha)});


    // Draw title
    TextDrawData titleTextDrawData;
    titleTextDrawData.position = sf::Vector2f(window.getSize().x / 2.0f, 300 * intScale);
    titleTextDrawData.size = 54;
    titleTextDrawData.text = "spacebuild";
    titleTextDrawData.colour = sf::Color(255, 255, 255);
    titleTextDrawData.centeredX = true;
    titleTextDrawData.centeredY = true;

    TextDraw::drawText(window, titleTextDrawData);

    // Buttons / UI
    switch (mainMenuState)
    {
        case MainMenuState::Main:
        {
            if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, window.getSize().y / 2.0f - 200.0f * intScale, 200 * intScale, 75 * intScale, "New"))
            {
                currentSaveName = "";
                worldSeed = "";
                mainMenuState = MainMenuState::StartingNew;
            }

            if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, window.getSize().y / 2.0f - 50 * intScale, 200 * intScale, 75 * intScale, "Load"))
            {
                currentSaveName = "";
                worldSeed = "";
                mainMenuState = MainMenuState::SelectingLoad;
                
                GameSaveIO io;
                saveFileNames = io.getSaveFiles();

                saveFilePage = 0;
            }

            if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, window.getSize().y / 2.0f + 100 * intScale, 200 * intScale, 75 * intScale, "Options"))
            {
                mainMenuState = MainMenuState::Options;
            }

            if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, window.getSize().y / 2.0f + 250 * intScale, 200 * intScale, 75 * intScale, "Exit"))
            {
                window.close();
                ImGui::SFML::Shutdown();
            }
            break;
        }
        case MainMenuState::StartingNew:
        {
            guiContext.createTextEnter(window.getSize().x / 2.0f - 200 * intScale, window.getSize().y / 2.0f - 100.0f * intScale,
                400 * intScale, 40 * intScale, "Name", &currentSaveName);

            guiContext.createTextEnter(window.getSize().x / 2.0f - 200 * intScale, window.getSize().y / 2.0f + 150 * intScale,
            400 * intScale, 40 * intScale, "Seed", &worldSeed);

            if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, window.getSize().y / 2.0f + 300 * intScale, 200 * intScale, 75 * intScale, "Start"))
            {
                if (!isStateTransitioning() && !currentSaveName.empty())
                {
                    startNewGame();
                }
            }
            
            if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, window.getSize().y - 150 * intScale, 200 * intScale, 75 * intScale, "Back"))
            {
                mainMenuState = MainMenuState::Main;
            }
            break;
        }
        case MainMenuState::SelectingLoad:
        {
            static constexpr int saveFilesPerPage = 5;

            for (int i = saveFilesPerPage * saveFilePage; i < std::min(static_cast<int>(saveFileNames.size()), saveFilesPerPage * (saveFilePage + 1)); i++)
            {
                const std::string& saveName = saveFileNames[i];

                if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, window.getSize().y / 2.0f - (150 - (i % saveFilesPerPage) * 100) * intScale,
                    200 * intScale, 75 * intScale, saveName))
                {
                    if (!isStateTransitioning())
                    {   
                        loadGame(saveName);
                    }
                }
            }

            // Text if no save files
            if (saveFileNames.size() <= 0)
            {
                TextDrawData textDrawData;
                textDrawData.text = "No save files found";
                textDrawData.position = sf::Vector2f(window.getSize().x / 2.0f, window.getSize().y / 2.0f - 150 * intScale);
                textDrawData.size = 24 * intScale;
                textDrawData.centeredX = true;
                textDrawData.centeredY = true;

                TextDraw::drawText(window, textDrawData);
            }

            // Create page scroll buttons if 
            if (saveFileNames.size() > saveFilesPerPage)
            {
                // Create page back button
                if (saveFilePage > 0)
                {
                    if (guiContext.createButton(window.getSize().x / 2.0f - 300 * intScale, window.getSize().y / 2.0f - 150 * intScale,
                        50 * intScale, 50 * intScale, "<"))
                    {
                        saveFilePage--;   
                    }
                }

                // Create page forward button
                if (saveFilePage < std::ceil(saveFileNames.size() / saveFilesPerPage))
                {
                    if (guiContext.createButton(window.getSize().x / 2.0f + 250 * intScale, window.getSize().y / 2.0f - 150 * intScale,
                        50 * intScale, 50 * intScale, ">"))
                    {
                        saveFilePage++;   
                    }
                }
            }

            if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, window.getSize().y - 150 * intScale, 200 * intScale, 75 * intScale, "Back"))
            {
                if (!isStateTransitioning())
                {
                    mainMenuState = MainMenuState::Main;
                }
            }
            break;
        }
        case MainMenuState::Options:
        {
            float musicVolume = Sounds::getMusicVolume();
            if (guiContext.createSlider(window.getSize().x / 2.0f - 200 * intScale, window.getSize().y / 2.0f, 400 * intScale, 15 * intScale,
                0.0f, 100.0f, &musicVolume, "Music Volume"))
            {
                Sounds::setMusicVolume(musicVolume);
            }

            if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, window.getSize().y - 150 * intScale, 200 * intScale, 75 * intScale, "Back"))
            {
                mainMenuState = MainMenuState::Main;
            }
            break;
        }
    }

    guiContext.draw(window);

    guiContext.endGUI();
}


// -- On Planet -- //

void Game::runOnPlanet(float dt)
{
    sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    bool shiftMode = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

    // Handle events
    for (auto event = sf::Event{}; window.pollEvent(event);)
    {
        handleEventsWindow(event);

        if (isStateTransitioning() || !player.isAlive())
        {
            continue;
        }
        
        if (worldMenuState == WorldMenuState::TravelSelect)
        {
            TravelSelectGUI::processEventGUI(event);
        }

        if (event.type == sf::Event::KeyPressed)
        {
            switch (worldMenuState)
            {
                case WorldMenuState::Main:
                {
                    if (event.key.code == sf::Keyboard::E)
                    {
                        worldMenuState = WorldMenuState::Inventory;
                        closeChest();
                    }
                    break;
                }
                case WorldMenuState::Inventory:
                {
                    if ((event.key.code == sf::Keyboard::E || event.key.code == sf::Keyboard::Escape) && worldMenuState == WorldMenuState::Inventory)
                    {
                        ItemType itemHeldBefore = InventoryGUI::getHeldItemType(inventory);

                        InventoryGUI::handleClose(inventory, chestDataPool.getChestDataPtr(openedChestID));
                        worldMenuState = WorldMenuState::Main;
                        closeChest();

                        if (itemHeldBefore != InventoryGUI::getHeldItemType(inventory))
                        {
                            changePlayerTool();
                        }
                    }
                    break;
                }
            }

            if (event.key.code == sf::Keyboard::Escape && worldMenuState == WorldMenuState::TravelSelect)
            {
                exitRocket();
            }
        }

        if (event.type == sf::Event::MouseButtonPressed && !ImGui::GetIO().WantCaptureMouse)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                switch (worldMenuState)
                {
                    case WorldMenuState::Main:
                    {
                        bool hotbarInteracted = InventoryGUI::handleLeftClickHotbar(mouseScreenPos);
                        if (!hotbarInteracted)
                        {
                            attemptUseTool();
                            attemptBuildObject();
                            attemptPlaceLand();
                            attemptUseBossSpawn();
                        }
                        else
                        {
                            changePlayerTool();
                        }
                        break;
                    }
                    case WorldMenuState::Inventory:
                    {
                        ItemType itemHeldBefore = InventoryGUI::getHeldItemType(inventory);
                        
                        if (InventoryGUI::isMouseOverUI(mouseScreenPos))
                        {
                            InventoryGUI::handleLeftClick(mouseScreenPos, shiftMode, inventory, armourInventory, chestDataPool.getChestDataPtr(openedChestID));
                        }
                        else
                        {
                            attemptUseTool();
                            attemptBuildObject();
                            attemptPlaceLand();
                            attemptUseBossSpawn();
                        }

                        if (itemHeldBefore != InventoryGUI::getHeldItemType(inventory))
                        {
                            changePlayerTool();
                        }
                        break;
                    }
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
                        if (InventoryGUI::isMouseOverUI(mouseScreenPos))
                        {
                            InventoryGUI::handleRightClick(mouseScreenPos, shiftMode, inventory, armourInventory, chestDataPool.getChestDataPtr(openedChestID));
                            changePlayerTool();
                        }
                        else
                        {
                            attemptObjectInteract();
                        }
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
                    // handleZoom(event.mouseWheelScroll.delta);
                    InventoryGUI::handleScrollHotbar(-event.mouseWheelScroll.delta);
                    changePlayerTool();
                    break;
            }
        }
    }


    //
    // -- UPDATING --
    //

    updateMusic(dt);

    // Update tweens
    floatTween.update(dt);

    // Update particles
    particleSystem.update(dt);

    HitMarkers::update(dt);

    Camera::update(player.getPosition(), mouseScreenPos, dt);

    updateDayNightCycle(dt);

    // Update depending on game state
    switch (gameState)
    {
        case GameState::OnPlanet:
            updateOnPlanet(dt);
            break;
        case GameState::InStructure:
            updateInStructure(dt);
            break;
    }

    Cursor::setCursorHidden(!player.canReachPosition(Cursor::getMouseWorldPos(window)));

    // Close chest if out of range
    checkChestOpenInRange();

    // Inventory GUI updating
    InventoryGUI::updateItemPopups(dt);

    if (player.isAlive())
    {
        switch (worldMenuState)
        {
            case WorldMenuState::Main:
            {
                InventoryGUI::updateHotbar(dt, mouseScreenPos);
                break;
            }
            case WorldMenuState::Inventory:
            {
                // Update inventory GUI available recipes if required, and animations
                InventoryGUI::updateAvailableRecipes(inventory, nearbyCraftingStationLevels);
                InventoryGUI::updateInventory(mouseScreenPos, dt, inventory, armourInventory, chestDataPool.getChestDataPtr(openedChestID));
                break;
            }
            case WorldMenuState::TravelSelect:
            {
                // std::vector<PlanetType> availableDestinations = getRocketAvailableDestinations();
                PlanetType selectedDestination;

                if (TravelSelectGUI::createGUI(window, selectedDestination))
                {
                    BuildableObject* rocketObject = chunkManager.getChunkObject(rocketEnteredReference.chunk, rocketEnteredReference.tile);

                    if (rocketObject)
                    {
                        destinationPlanet = selectedDestination;
                        worldMenuState = WorldMenuState::FlyingRocket;
                        rocketObject->triggerBehaviour(*this, ObjectBehaviourTrigger::RocketFlyUp);
                    }
                }
            }
        }
    }


    //
    // -- DRAWING --
    //

    window.clear();

    // Draw depending on game state
    switch (gameState)
    {
        case GameState::OnPlanet:
            drawOnPlanet(dt);
            break;
        case GameState::InStructure:
            drawInStructure(dt);
            break;
    }

    if (player.isAlive())
    {
        switch (worldMenuState)
        {
            case WorldMenuState::Main:
                InventoryGUI::drawHotbar(window, mouseScreenPos, inventory);
                InventoryGUI::drawItemPopups(window);
                HealthGUI::drawHealth(window, spriteBatch, player.getHealth(), player.getMaxHealth(), gameTime);
                break;
            
            case WorldMenuState::Inventory:
                InventoryGUI::draw(window, gameTime, mouseScreenPos, inventory, armourInventory, chestDataPool.getChestDataPtr(openedChestID));
                HealthGUI::drawHealth(window, spriteBatch, player.getHealth(), player.getMaxHealth(), gameTime);
                break;
            
            case WorldMenuState::TravelSelect:
                TravelSelectGUI::drawGUI(window);
                break;
        }
    }
    else
    {
        HealthGUI::drawDeadPrompt(window);
    }

    spriteBatch.endDrawing(window);
}

void Game::updateOnPlanet(float dt)
{
    sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    int worldSize = chunkManager.getWorldSize();

    // Update cursor
    Cursor::updateTileCursor(window, dt, chunkManager, player.getCollisionRect(), InventoryGUI::getHeldItemType(inventory), player.getTool());

    // Update player
    bool wrappedAroundWorld = false;
    sf::Vector2f wrapPositionDelta;

    if (!isStateTransitioning())
        player.update(dt, Cursor::getMouseWorldPos(window), chunkManager, worldSize, wrappedAroundWorld, wrapPositionDelta);

    // Handle world wrapping for camera and cursor, if player wrapped around
    if (wrappedAroundWorld)
    {
        Camera::handleWorldWrap(wrapPositionDelta);
        Cursor::handleWorldWrap(wrapPositionDelta);
        handleOpenChestPositionWorldWrap(wrapPositionDelta);
        chunkManager.reloadChunks();

        // Wrap bosses
        bossManager.handleWorldWrap(wrapPositionDelta);

        // Wrap projectiles
        projectileManager.handleWorldWrap(wrapPositionDelta);

        // Wrap hit markers
        HitMarkers::handleWorldWrap(wrapPositionDelta);
    }

    // Update (loaded) chunks
    chunkManager.updateChunks();
    chunkManager.updateChunksObjects(*this, dt);
    chunkManager.updateChunksEntities(dt, projectileManager, inventory);
    
    // Get nearby crafting stations
    nearbyCraftingStationLevels = chunkManager.getNearbyCraftingStationLevels(player.getChunkInside(worldSize), player.getChunkTileInside(worldSize), 4);

    // Update bosses
    bossManager.update(*this, projectileManager, inventory, player, dt);

    // Update projectiles
    projectileManager.update(dt);

    if (!isStateTransitioning() && !player.isInRocket() && player.isAlive())
    {
        testEnterStructure();
    }

    if (travelPlanetTrigger)
    {
        travelToPlanet(destinationPlanet);
    }
}

void Game::drawOnPlanet(float dt)
{
    // Get world objects
    std::vector<WorldObject*> worldObjects = chunkManager.getChunkObjects();
    std::vector<WorldObject*> entities = chunkManager.getChunkEntities();
    worldObjects.insert(worldObjects.end(), entities.begin(), entities.end());
    worldObjects.push_back(&player);

    drawWorld(dt, worldObjects);
    drawLighting(dt, worldObjects);

    // UI
    sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    Cursor::drawCursor(window);

    if (player.getTool() < 0)
    {
        ObjectType placeObject = InventoryGUI::getHeldObjectType(inventory);

        if (placeObject >= 0)
        {
            drawGhostPlaceObjectAtCursor(placeObject);
        }

        // Draw land to place if held
        if ((InventoryGUI::heldItemPlacesLand(inventory)))
        {
            drawGhostPlaceLandAtCursor();
        }
    }

    HitMarkers::draw(window);

    bossManager.drawStatsAtCursor(window, mouseScreenPos);
}

void Game::drawWorld(float dt, std::vector<WorldObject*>& worldObjects)
{
    // Draw all world onto texture for lighting
    worldTexture.create(window.getSize().x, window.getSize().y);
    worldTexture.clear();

    // Draw water
    chunkManager.drawChunkWater(worldTexture, gameTime);

    std::sort(worldObjects.begin(), worldObjects.end(), [](WorldObject* a, WorldObject* b)
    {
        if (a->getDrawLayer() != b->getDrawLayer()) return a->getDrawLayer() > b->getDrawLayer();
        if (a->getPosition().y == b->getPosition().y) return a->getPosition().x < b->getPosition().x;
        return a->getPosition().y < b->getPosition().y;
    });

    spriteBatch.beginDrawing();

    // Draw terrain
    chunkManager.drawChunkTerrain(worldTexture, spriteBatch, gameTime);

    // Draw objects
    for (WorldObject* worldObject : worldObjects)
    {
        worldObject->draw(worldTexture, spriteBatch, dt, gameTime, chunkManager.getWorldSize(), {255, 255, 255, 255});
    }

    // Draw bosses
    bossManager.draw(worldTexture, spriteBatch);

    // Draw projectiles
    projectileManager.drawProjectiles(worldTexture, spriteBatch);

    spriteBatch.endDrawing(worldTexture);

    worldTexture.display();
}

void Game::drawLighting(float dt, std::vector<WorldObject*>& worldObjects)
{
    unsigned char ambientRedLight = (1.f - worldDarkness) * 255.0f;
    unsigned char ambientGreenLight = (1.f - worldDarkness * 0.97f) * 255.0f;
    unsigned char ambientBlueLight = (1.f - worldDarkness * 0.93f) * 255.0f;

    sf::Vector2i chunksSizeInView = chunkManager.getChunksSizeInView();
    sf::Vector2f topLeftChunkPos = chunkManager.topLeftChunkPosInView();
    
    // Draw light sources on light texture
    sf::RenderTexture lightTexture;
    lightTexture.create(chunksSizeInView.x * CHUNK_TILE_SIZE * TILE_LIGHTING_RESOLUTION, chunksSizeInView.y * CHUNK_TILE_SIZE * TILE_LIGHTING_RESOLUTION);

    // Prepare lighting engine
    lightingEngine.resize(chunksSizeInView.x * CHUNK_TILE_SIZE * TILE_LIGHTING_RESOLUTION, chunksSizeInView.y * CHUNK_TILE_SIZE * TILE_LIGHTING_RESOLUTION);

    // player.drawLightMask(lightTexture);

    for (WorldObject* worldObject : worldObjects)
    {
        // worldObject->drawLightMask(lightTexture);
        worldObject->createLightSource(lightingEngine, topLeftChunkPos);
    }

    lightingEngine.calculateLighting();

    lightTexture.clear({ambientRedLight, ambientGreenLight, ambientBlueLight, 255});

    // draw from lighting engine
    lightingEngine.drawLighting(lightTexture, sf::Color(255, 220, 140));

    lightTexture.display();
    lightTexture.setSmooth(true);

    sf::Sprite lightTextureSprite(lightTexture.getTexture());
    // lightTextureSprite.setColor(sf::Color(255, 255, 255, 255));

    lightTextureSprite.setPosition(Camera::worldToScreenTransform(topLeftChunkPos));
    lightTextureSprite.setScale(sf::Vector2f(ResolutionHandler::getScale(), ResolutionHandler::getScale()) * TILE_SIZE_PIXELS_UNSCALED / static_cast<float>(TILE_LIGHTING_RESOLUTION));

    worldTexture.draw(lightTextureSprite, sf::BlendMultiply);

    worldTexture.display();

    sf::Sprite worldTextureSprite(worldTexture.getTexture());
    window.draw(worldTextureSprite);
}

void Game::updateDayNightCycle(float dt)
{
    // Update day / night cycle
    dayNightToggleTimer += dt;

    if (dayNightToggleTimer >= DAY_LENGTH)
    {
        dayNightToggleTimer = 0.0f;
        if (isDay) floatTween.startTween(&worldDarkness, 0.0f, 0.95f, 7, TweenTransition::Sine, TweenEasing::EaseInOut);
        else floatTween.startTween(&worldDarkness, 0.95f, 0.0f, 7, TweenTransition::Sine, TweenEasing::EaseInOut);
        isDay = !isDay;
    }
}

// Structure
void Game::testEnterStructure()
{
    StructureEnterEvent enterEvent;
    if (!chunkManager.isPlayerInStructureEntrance(player.getPosition(), enterEvent))
        return;
    
    // Structure has been entered

    // Create room data
    if (enterEvent.enteredStructure->getStructureID() == 0xFFFFFFFF)
    {
        structureEnteredID = structureRoomPool.createRoom(enterEvent.enteredStructure->getStructureType(), chestDataPool);
        enterEvent.enteredStructure->setStructureID(structureEnteredID);
    }
    else
    {
        // Get ID from structure as room has previously been initialised
        structureEnteredID = enterEvent.enteredStructure->getStructureID();
    }

    structureEnteredPos.x = (std::floor(enterEvent.entrancePosition.x / TILE_SIZE_PIXELS_UNSCALED) + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
    structureEnteredPos.y = (std::floor(enterEvent.entrancePosition.y / TILE_SIZE_PIXELS_UNSCALED) + 1.5f) * TILE_SIZE_PIXELS_UNSCALED;

    // changeState(GameState::InStructure);
    startChangeStateTransition(GameState::InStructure);
}

void Game::testExitStructure()
{
    const Room& structureRoom = structureRoomPool.getRoom(structureEnteredID);
    
    if (!structureRoom.isPlayerInExit(player.getPosition()))
        return;

    // changeState(GameState::OnPlanet);
    startChangeStateTransition(GameState::OnPlanet);
}

// Rocket
void Game::enterRocket(RocketObject& rocket)
{
    rocketEnteredReference.chunk = rocket.getChunkInside(chunkManager.getWorldSize());
    rocketEnteredReference.tile = rocket.getChunkTileInside(chunkManager.getWorldSize());

    // Save just before enter
    saveGame(true);

    worldMenuState = WorldMenuState::TravelSelect;

    TravelSelectGUI::setAvailableDestinations(rocket.getRocketAvailableDestinations(chunkManager.getPlanetType()));

    player.enterRocket(rocket.getRocketPosition());
}

void Game::exitRocket()
{
    worldMenuState = WorldMenuState::Main;

    // Exit rocket object
    BuildableObject* rocketObject = chunkManager.getChunkObject(rocketEnteredReference.chunk, rocketEnteredReference.tile);

    if (rocketObject)
    {
        rocketObject->triggerBehaviour(*this, ObjectBehaviourTrigger::RocketExit);
    }

    player.exitRocket();
}

void Game::enterIncomingRocket(RocketObject& rocket)
{
    player.enterRocket(rocket.getRocketPosition());
}

void Game::rocketFinishedUp(RocketObject& rocket)
{
    travelPlanetTrigger = true;
}

void Game::rocketFinishedDown(RocketObject& rocket)
{
    exitRocket();
}


// -- In Structure -- //

void Game::updateInStructure(float dt)
{
    sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    Room& structureRoom = structureRoomPool.getRoom(structureEnteredID);

    Cursor::updateTileCursorInRoom(window, dt, structureRoom.getObjectGrid(), InventoryGUI::getHeldItemType(inventory), player.getTool());

    if (!isStateTransitioning())
        player.updateInStructure(dt, Cursor::getMouseWorldPos(window), structureRoom);

    // Update room objects
    structureRoom.updateObjects(*this, dt);

    // Continue to update objects and entities in world
    chunkManager.updateChunksObjects(*this, dt);
    chunkManager.updateChunksEntities(dt, projectileManager, inventory);

    if (!isStateTransitioning())
        testExitStructure();
}

void Game::drawInStructure(float dt)
{
    const Room& structureRoom = structureRoomPool.getRoom(structureEnteredID);
    structureRoom.draw(window);

    std::vector<const WorldObject*> worldObjects = structureRoom.getObjects();
    worldObjects.push_back(&player);

    std::sort(worldObjects.begin(), worldObjects.end(), [](const WorldObject* a, const WorldObject* b)
    {
        if (a->getDrawLayer() != b->getDrawLayer()) return a->getDrawLayer() > b->getDrawLayer();
        if (a->getPosition().y == b->getPosition().y) return a->getPosition().x < b->getPosition().x;
        return a->getPosition().y < b->getPosition().y;
    });

    for (const WorldObject* object : worldObjects)
    {
        object->draw(window, spriteBatch, dt, gameTime, chunkManager.getWorldSize(), sf::Color(255, 255, 255));
    }

    spriteBatch.endDrawing(window);

    Cursor::drawCursor(window);
}


// -- Tools / interaction -- //

void Game::changePlayerTool()
{
    // Get currently selected tool in inventory and hotbar
    ToolType heldTool = InventoryGUI::getHeldToolType(inventory);

    // Get tool currently held by player

    // If inventory tool is selected, override hotbar tool
    if (heldTool >= 0)
    {
        player.setTool(heldTool);
    }
    else
    {
        player.setTool(-1);
    }
}

void Game::attemptUseTool()
{
    if (player.getTool() < 0)
        return;

    if (player.isUsingTool())
        return;
    
    if (player.isInRocket())
        return;

    // if (gameState != GameState::OnPlanet)
        // return;
    
    // Get tool data for tool behaviour, to choose which tool use function
    const ToolData& toolData = ToolDataLoader::getToolData(player.getTool());

    switch(toolData.toolBehaviourType)
    {
        case ToolBehaviourType::Pickaxe:
            attemptUseToolPickaxe();
            break;
        case ToolBehaviourType::FishingRod:
            attemptUseToolFishingRod();
            break;
        default:
            attemptUseToolWeapon();
            break;
    }
}

void Game::attemptUseToolPickaxe()
{
    sf::Vector2f mouseWorldPos = Cursor::getMouseWorldPos(window);
    
    // Swing pickaxe
    player.useTool(projectileManager, inventory, mouseWorldPos);

    if (gameState != GameState::OnPlanet)
        return;

    if (!player.canReachPosition(mouseWorldPos))
        return;

    // Get current tool damage amount
    ToolType currentTool = player.getTool();

    const ToolData& toolData = ToolDataLoader::getToolData(currentTool);

    // Entity* selectedEntity = chunkManager.getSelectedEntity(Cursor::getSelectedChunk(chunkManager.getWorldSize()), mouseWorldPos);
    // if (selectedEntity != nullptr)
    // {
        // selectedEntity->damage(toolData.damage, inventory);
    // }
    // else
    // {
    bool canDestroyObject = chunkManager.canDestroyObject(Cursor::getSelectedChunk(chunkManager.getWorldSize()),
                                                    Cursor::getSelectedChunkTile(),
                                                    player.getCollisionRect());

    if (!canDestroyObject)
        return;

    BuildableObject* selectedObject = chunkManager.getChunkObject(Cursor::getSelectedChunk(
        chunkManager.getWorldSize()), Cursor::getSelectedChunkTile());

    if (selectedObject)
    {
        selectedObject->damage(toolData.damage, *this, inventory);
    }
    // }
}

void Game::attemptUseToolFishingRod()
{
    if (gameState != GameState::OnPlanet)
        return;

    // Check if fish is biting line first - if so, reel in fishing rod and catch fish
    if (player.isFishBitingLine())
    {
        sf::Vector2i fishedTile = player.reelInFishingRod();
        catchRandomFish(fishedTile);
        return;
    }

    sf::Vector2f mouseWorldPos = Cursor::getMouseWorldPos(window);

    if (!player.canReachPosition(mouseWorldPos))
    {
        player.reelInFishingRod();
        return;
    }
    
    // Determine whether can fish at selected tile
    ChunkPosition selectedChunk = Cursor::getSelectedChunk(chunkManager.getWorldSize());
    sf::Vector2i selectedTile = Cursor::getSelectedChunkTile();

    // Test whether can fish on selected tile
    // Must have no object + be water
    BuildableObject* selectedObject = chunkManager.getChunkObject(selectedChunk, selectedTile);
    int tileType = chunkManager.getChunkTileType(selectedChunk, selectedTile);

    if (selectedObject || tileType > 0)
    {
        player.reelInFishingRod();
        return;
    }
    
    // Swing fishing rod
    player.useTool(projectileManager, inventory, mouseWorldPos);

    player.swingFishingRod(mouseWorldPos, Cursor::getSelectedWorldTile(chunkManager.getWorldSize()));
}

void Game::attemptUseToolWeapon()
{
    sf::Vector2f mouseWorldPos = Cursor::getMouseWorldPos(window);

    player.useTool(projectileManager, inventory, mouseWorldPos);
}

void Game::catchRandomFish(sf::Vector2i fishedTile)
{
    const BiomeGenData* biomeGenData = Chunk::getBiomeGenAtWorldTile(fishedTile, chunkManager.getWorldSize(), chunkManager.getBiomeNoise(), chunkManager.getPlanetType());

    // Check for nullptr
    if (!biomeGenData)
        return;
    
    // Randomise catch
    float randomChance = Helper::randInt(0, 10000) / 10000.0f;
    float cumulativeChance = 0.0f;
    for (const FishCatchData& fishCatchData : biomeGenData->fishCatchDatas)
    {
        cumulativeChance += fishCatchData.chance;
        
        if (cumulativeChance >= randomChance)
        {
            // Add fish / catch
            inventory.addItem(fishCatchData.itemCatch, fishCatchData.count);
            InventoryGUI::pushItemPopup(ItemCount(fishCatchData.itemCatch, fishCatchData.count));
            break;
        }
    }
}

void Game::attemptObjectInteract()
{
    // Get mouse position in screen space and world space
    sf::Vector2f mouseWorldPos = Cursor::getMouseWorldPos(window);

    if (!player.canReachPosition(mouseWorldPos))
        return;
    
    BuildableObject* selectedObject = getSelectedObjectFromChunkOrRoom();

    if (selectedObject)
    {
        selectedObject->interact(*this);
    }
}

void Game::attemptBuildObject()
{
    if (gameState != GameState::OnPlanet)
        return;
    
    if (player.isInRocket())
        return;

    ObjectType objectType = InventoryGUI::getHeldObjectType(inventory);
    // bool placeFromHotbar = false;

    if (objectType < 0)
        return;

    // // Do not build if holding tool in inventory
    // if (player.getTool() >= 0)
    //     return;


    // bool canAfford = Inventory::canBuildObject(objectType);
    bool canPlace = chunkManager.canPlaceObject(Cursor::getSelectedChunk(chunkManager.getWorldSize()),
                                                Cursor::getSelectedChunkTile(),
                                                objectType,
                                                player.getCollisionRect());

    bool inRange = player.canReachPosition(Cursor::getMouseWorldPos(window));

    if (canPlace && inRange)
    {
        // Remove object from being held
        // if (placeFromHotbar)
        // {
        //     InventoryGUI::placeHotbarObject(inventory);
        // }
        // else
        // {
        InventoryGUI::subtractHeldItem(inventory);
        // }

        // Play build sound
        int soundChance = rand() % 2;
        SoundType buildSound = SoundType::CraftBuild1;
        if (soundChance == 1) buildSound = SoundType::CraftBuild2;

        Sounds::playSound(buildSound, 60.0f);

        // Build object
        chunkManager.setObject(Cursor::getSelectedChunk(chunkManager.getWorldSize()), Cursor::getSelectedChunkTile(), objectType);
    }
}

void Game::attemptPlaceLand()
{
    if (gameState != GameState::OnPlanet)
        return;
    
    if (player.isInRocket())
        return;
    
    if (!InventoryGUI::heldItemPlacesLand(inventory))
        return;

    // Do not build if holding tool in inventory
    if (player.getTool() >= 0)
        return;

    // bool placeFromHotbar = false;

    // if (!InventoryGUI::heldItemPlacesLand(inventory))
    // {
    //     if (InventoryGUI::hotbarItemPlacesLand(inventory))
    //     {
    //         placeFromHotbar = true;
    //     }
    //     else
    //     {
    //         return;
    //     }
    // }
    
    if (!chunkManager.canPlaceLand(Cursor::getSelectedChunk(chunkManager.getWorldSize()), Cursor::getSelectedChunkTile()))
        return;
    
    if (!player.canReachPosition(Cursor::getMouseWorldPos(window)))
        return;
    
    // Place land
    chunkManager.placeLand(Cursor::getSelectedChunk(chunkManager.getWorldSize()), Cursor::getSelectedChunkTile());

    // Play build sound
    int soundChance = rand() % 2;
    SoundType buildSound = SoundType::CraftBuild1;
    if (soundChance == 1) buildSound = SoundType::CraftBuild2;

    Sounds::playSound(buildSound, 60.0f);

    // Subtract from land held
    // if (placeFromHotbar)
    // {
    //     InventoryGUI::placeHotbarObject(inventory);
    // }
    // else
    // {
    InventoryGUI::subtractHeldItem(inventory);
    // }
}

void Game::attemptUseBossSpawn()
{
    if (gameState != GameState::OnPlanet)
    {
        return;
    }

    if (player.isInRocket())
    {
        return;
    }

    ItemType heldItemType = InventoryGUI::getHeldItemType(inventory);

    if (heldItemType < 0)
    {
        return;
    }

    const ItemData& itemData = ItemDataLoader::getItemData(heldItemType);

    if (!itemData.bossSummonData.has_value())
    {
        return;
    }

    if (itemData.bossSummonData->useAtNight && isDay)
    {
        return;
    }

    // Take boss summon item
    InventoryGUI::subtractHeldItem(inventory);

    // Summon boss
    sf::Vector2f spawnPos;
    spawnPos.x = player.getPosition().x - 400.0f;
    spawnPos.y = player.getPosition().y - 400.0f;

    bossManager.createBoss(itemData.bossSummonData->bossName, spawnPos);
}

void Game::drawGhostPlaceObjectAtCursor(ObjectType object)
{
    // Draw object to be placed if held
    bool canPlace = chunkManager.canPlaceObject(Cursor::getSelectedChunk(chunkManager.getWorldSize()),
                                                Cursor::getSelectedChunkTile(),
                                                object,
                                                player.getCollisionRect());

    bool inRange = player.canReachPosition(Cursor::getMouseWorldPos(window));

    sf::Color drawColor(255, 0, 0, 180);
    if (canPlace && inRange)
        drawColor = sf::Color(0, 255, 0, 180);
    
    BuildableObject objectGhost(Cursor::getLerpedSelectPos() + sf::Vector2f(TILE_SIZE_PIXELS_UNSCALED / 2.0f, TILE_SIZE_PIXELS_UNSCALED / 2.0f), object, false);

    objectGhost.draw(window, spriteBatch, 0.0f, 0, chunkManager.getWorldSize(), drawColor);

    spriteBatch.endDrawing(window);
}

void Game::drawGhostPlaceLandAtCursor()
{
    sf::Vector2f tileWorldPosition = Cursor::getLerpedSelectPos();
    int worldSize = chunkManager.getWorldSize();

    // Change color depending on whether can place land or not
    sf::Color landGhostColor(255, 0, 0, 180);
    if (chunkManager.canPlaceLand(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile()))
    {
        landGhostColor = sf::Color(0, 255, 0, 180);
    }

    float scale = ResolutionHandler::getScale();

    // Sample noise to select correct tile to draw
    const BiomeGenData* biomeGenData = Chunk::getBiomeGenAtWorldTile(
        Cursor::getSelectedWorldTile(worldSize), worldSize, chunkManager.getBiomeNoise(), chunkManager.getPlanetType()
        );
    
    // Check for nullptr (shouldn't happen)
    if (!biomeGenData)
        return;
    
    // Get texture offset for tilemap
    sf::Vector2i tileMapTextureOffset = biomeGenData->tileGenDatas[0].tileMap.textureOffset;

    // Create texture rect of centre tile from tilemap
    sf::IntRect textureRect(tileMapTextureOffset.x + 16, tileMapTextureOffset.y + 16, 16, 16);

    // Draw tile at screen position
    TextureManager::drawSubTexture(window, {
        .type = TextureType::GroundTiles,
        .position = Camera::worldToScreenTransform(tileWorldPosition),
        .scale = {scale, scale},
        .colour = landGhostColor
    }, textureRect);
}

BuildableObject* Game::getSelectedObjectFromChunkOrRoom()
{
    sf::Vector2f mouseWorldPos = Cursor::getMouseWorldPos(window);

    if (gameState == GameState::OnPlanet)
    {
        BuildableObject* selectedObject = chunkManager.getChunkObject(
            Cursor::getSelectedChunk(chunkManager.getWorldSize()), Cursor::getSelectedChunkTile());
        
        return selectedObject;
    }
    else if (gameState == GameState::InStructure)
    {
        Room& structureRoom = structureRoomPool.getRoom(structureEnteredID);
        
        return structureRoom.getObject(mouseWorldPos);
    }

    return nullptr;
}

BuildableObject* Game::getObjectFromChunkOrRoom(ObjectReference objectReference)
{
    BuildableObject* objectPtr = nullptr;

    if (gameState == GameState::OnPlanet)
    {
        return chunkManager.getChunkObject(objectReference.chunk, objectReference.tile);
    }
    else if (gameState == GameState::InStructure)
    {
        Room& structureRoom = structureRoomPool.getRoom(structureEnteredID);
        return structureRoom.getObject(objectReference.tile);
    }

    return nullptr;
}


// -- Inventory / Chests -- //

void Game::giveStartingInventory()
{
    inventory.addItem(ItemDataLoader::getItemTypeFromName("Wooden Pickaxe"), 1);

    changePlayerTool();
}

void Game::openChest(ChestObject& chest)
{
    openedChestID = chest.getChestID();

    openedChestPos = chest.getPosition();

    InventoryGUI::chestOpened(chestDataPool.getChestDataPtr(openedChestID));

    worldMenuState = WorldMenuState::Inventory;
}

uint16_t Game::getOpenChestID()
{
    return openedChestID;
}

ChestDataPool& Game::getChestDataPool()
{
    return chestDataPool;
}

void Game::checkChestOpenInRange()
{
    if (openedChestID == 0xFFFF)
        return;

    if (!player.canReachPosition(openedChestPos))
    {
        closeChest();
    }
}

void Game::handleOpenChestPositionWorldWrap(sf::Vector2f positionDelta)
{
    openedChestPos += positionDelta;
}

void Game::closeChest()
{
    InventoryGUI::chestClosed();

    openedChestID = 0xFFFF;
    openedChestPos = sf::Vector2f(0, 0);
}


// -- Planet travelling -- //

void Game::travelToPlanet(PlanetType planetType)
{
    //exitRocket();

    travelPlanetTrigger = false;

    player.exitRocket();

    resetChestDataPool();
    resetStructureRoomPool();
    bossManager.clearBosses();

    if (!loadPlanet(planetType))
    {
        initialiseNewPlanet(planetType, true);   
    }

    Camera::instantUpdate(player.getPosition());

    chunkManager.updateChunks();

    // Start rocket flying downwards
    BuildableObject* rocketObject = chunkManager.getChunkObject(rocketEnteredReference.chunk, rocketEnteredReference.tile);
    if (rocketObject)
    {
        rocketObject->triggerBehaviour(*this, ObjectBehaviourTrigger::RocketFlyDown);
    }
    // RocketObject* rocket = static_cast<RocketObject*>(chunkManager.getChunkObject(rocketObject.chunk, rocketObject.tile));

    // player.enterRocket(rocket->getRocketPosition());
    // startFlyingRocket(chunkManager.getPlanetType(), true);

    Camera::instantUpdate(player.getPosition());
}

void Game::initialiseNewPlanet(PlanetType planetType, bool placeRocket)
{
    chunkManager.setPlanetType(planetType);

    ChunkPosition playerSpawnChunk = chunkManager.findValidSpawnChunk(2);

    sf::Vector2f playerSpawnPos;
    playerSpawnPos.x = playerSpawnChunk.x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    playerSpawnPos.y = playerSpawnChunk.y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    player.setPosition(playerSpawnPos);
    
    // Place rocket
    if (placeRocket)
    {
        Camera::instantUpdate(player.getPosition());

        chunkManager.updateChunks();

        // Ennsure spawn chunk does not have structure
        chunkManager.regenerateChunkWithoutStructure(playerSpawnChunk);

        chunkManager.setObject(playerSpawnChunk, sf::Vector2i(0, 0), ObjectDataLoader::getObjectTypeFromName("Rocket Launch Pad"));
        rocketEnteredReference.chunk = playerSpawnChunk;
        rocketEnteredReference.tile = sf::Vector2i(0, 0);
    }
}

void Game::resetChestDataPool()
{
    chestDataPool = ChestDataPool();
}

void Game::resetStructureRoomPool()
{
    structureRoomPool = RoomPool();
}


// -- Game State / Transition -- //

void Game::updateStateTransition(float dt)
{
    transitionGameStateTimer -= dt;
    if (transitionGameStateTimer <= 0)
    {
        transitionGameStateTimer = 0;

        // Change state
        changeState(destinationGameState);
    }
}

void Game::drawStateTransition()
{
    sf::RectangleShape fadeRectangle(static_cast<sf::Vector2f>(window.getSize()));

    // Calculate alpha
    float alpha = 1.0f - transitionGameStateTimer / TRANSITION_STATE_FADE_TIME;
    fadeRectangle.setFillColor(sf::Color(0, 0, 0, 255 * alpha));

    window.draw(fadeRectangle);
}

bool Game::isStateTransitioning()
{
    return (gameState != destinationGameState);
}

void Game::startChangeStateTransition(GameState newState)
{
    destinationGameState = newState;

    transitionGameStateTimer = TRANSITION_STATE_FADE_TIME;
}

void Game::changeState(GameState newState)
{
    switch (newState)
    {
        case GameState::InStructure:
        {
            closeChest();
            nearbyCraftingStationLevels.clear();
            
            sf::Vector2f roomEntrancePos = structureRoomPool.getRoom(structureEnteredID).getEntrancePosition();

            player.setPosition(roomEntrancePos);
            Camera::instantUpdate(player.getPosition());

            player.enterStructure();
            break;
        }
        case GameState::OnPlanet:
        {
            if (gameState == GameState::InStructure)
            {
                // Exit structure
                structureEnteredID = 0xFFFFFFFF;

                player.setPosition(structureEnteredPos);
                Camera::instantUpdate(player.getPosition());
            }
            break;
        }
    }

    gameState = newState;
}

void Game::setWorldSeedFromInput()
{
    // Get seed from input
    int seed = 0;
    if (worldSeed.empty())
    {
        seed = rand();
    }
    else
    {
        // Compute simple hash of seed
        seed = 0x55555555;
        for (char c : worldSeed)
        {
            seed ^= c;
            seed = seed << 5;
        }
    }

    chunkManager.setSeed(seed);
}


// -- Save / load -- //

void Game::startNewGame()
{
    setWorldSeedFromInput();

    initialiseNewPlanet(PlanetGenDataLoader::getPlanetTypeFromName("Earthlike"));

    Camera::instantUpdate(player.getPosition());

    chunkManager.updateChunks();

    startChangeStateTransition(GameState::OnPlanet);
}

bool Game::saveGame(bool gettingInRocket)
{
    GameSaveIO io(currentSaveName);

    PlayerGameSave playerGameSave;
    playerGameSave.seed = chunkManager.getSeed();
    playerGameSave.planetType = chunkManager.getPlanetType();
    // playerGameSave.playerPos = player.getPosition();
    playerGameSave.inventory = inventory;
    playerGameSave.armourInventory = armourInventory;
    playerGameSave.time = dayNightToggleTimer;
    playerGameSave.isDay = isDay;

    PlanetGameSave planetGameSave;
    planetGameSave.playerLastPos = player.getPosition();
    planetGameSave.chunks = chunkManager.getChunkPODs();
    planetGameSave.chestDataPool = chestDataPool;
    planetGameSave.structureRoomPool = structureRoomPool;
    
    if (gettingInRocket)
    {
        planetGameSave.rocketObjectUsed = rocketEnteredReference;
    }

    io.write(playerGameSave, planetGameSave);

    return true;
}

bool Game::loadGame(const std::string& saveName)
{
    GameSaveIO io(saveName);

    PlayerGameSave playerGameSave;
    PlanetGameSave planetGameSave;

    if (!io.load(playerGameSave, planetGameSave))
    {
        std::cout << "Failed to load game\n";
        return false;
    }

    closeChest();
    
    chunkManager.setSeed(playerGameSave.seed);
    chunkManager.setPlanetType(playerGameSave.planetType);
    inventory = playerGameSave.inventory;
    armourInventory = playerGameSave.armourInventory;
    player.setPosition(planetGameSave.playerLastPos);
    dayNightToggleTimer = playerGameSave.time;
    isDay = playerGameSave.isDay;

    if (isDay) worldDarkness = 0.0f;
    else worldDarkness = 0.95f;

    changePlayerTool();

    chunkManager.loadFromChunkPODs(planetGameSave.chunks);
    chestDataPool = planetGameSave.chestDataPool;
    structureRoomPool = planetGameSave.structureRoomPool;

    Camera::instantUpdate(player.getPosition());

    chunkManager.updateChunks();

    // Load successful, set save name as current save and start state transition
    currentSaveName = saveName;
    startChangeStateTransition(GameState::OnPlanet);

    return true;
}

bool Game::loadPlanet(PlanetType planetType)
{
    GameSaveIO io(currentSaveName);

    PlanetGameSave planetGameSave;

    if (!io.loadPlanet(planetType, planetGameSave))
    {
        return false;
    }

    chunkManager.setPlanetType(planetType);

    player.setPosition(planetGameSave.playerLastPos);
    chunkManager.loadFromChunkPODs(planetGameSave.chunks);
    chestDataPool = planetGameSave.chestDataPool;
    structureRoomPool = planetGameSave.structureRoomPool;

    // assert(planetGameSave.rocketObjectUsed.has_value());

    if (planetGameSave.rocketObjectUsed.has_value())
    {
        rocketEnteredReference = planetGameSave.rocketObjectUsed.value();
    }
    else
    {
        return false;
    }

    // rocketObject = planetGameSave.rocketObjectUsed.value();

    return true;
}



// -- Window -- //

void Game::handleZoom(int zoomChange)
{
    float beforeScale = ResolutionHandler::getScale();
    ResolutionHandler::changeZoom(zoomChange);
    
    float afterScale = ResolutionHandler::getScale();

    Camera::handleScaleChange(beforeScale, afterScale, player.getPosition());
}

void Game::handleEventsWindow(sf::Event& event)
{
    if (event.type == sf::Event::Closed)
    {
        window.close();
        ImGui::SFML::Shutdown();
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

        if (event.key.code == sf::Keyboard::F1)
        {
            DebugOptions::debugOptionsMenuOpen = !DebugOptions::debugOptionsMenuOpen;
            return;
        }
    }

    // ImGui
    ImGui::SFML::ProcessEvent(window, event);
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
    window.setMouseCursorVisible(false);

    handleWindowResize(sf::Vector2u(videoMode.width, videoMode.height));
}

void Game::handleWindowResize(sf::Vector2u newSize)
{
    unsigned int newWidth = newSize.x;
    unsigned int newHeight = newSize.y;

    if (!fullScreen)
    {
        newWidth = std::max(newSize.x, 1280U);
        newHeight = std::max(newSize.y, 720U);
    }

    window.setSize(sf::Vector2u(newWidth, newHeight));

    view.setSize(newWidth, newHeight);
    view.setCenter({newWidth / 2.0f, newHeight / 2.0f});

    // float beforeScale = ResolutionHandler::getScale();

    ResolutionHandler::setResolution({newWidth, newHeight});

    Camera::instantUpdate(player.getPosition());

    // float afterScale = ResolutionHandler::getScale();

    // if (beforeScale != afterScale)
        // Camera::handleScaleChange(beforeScale, afterScale, player.getPosition());
}


// -- Misc -- //

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
    std::vector<sf::Uint8> noiseData(waterNoiseSize * waterNoiseSize * 4);
    std::vector<sf::Uint8> noiseTwoData(waterNoiseSize * waterNoiseSize * 4);

    // Sample noise data
    for (int y = 0; y < waterNoiseSize; y++)
    {
        for (int x = 0; x < waterNoiseSize; x++)
        {
            int index = (y * waterNoiseSize + x) * 4;

            float noiseValue = waterNoise.GetNoiseSeamless2D(x, y, waterNoiseSize, waterNoiseSize);
            noiseValue = FastNoise::Normalise(noiseValue);
            noiseData[index] = noiseValue * 255;
            noiseData[index + 1] = noiseValue * 255;
            noiseData[index + 2] = noiseValue * 255;
            noiseData[index + 3] = 255;

            noiseValue = waterNoiseTwo.GetNoiseSeamless2D(x, y, waterNoiseSize, waterNoiseSize);
            noiseValue = FastNoise::Normalise(noiseValue);
            noiseTwoData[index + 0] = noiseValue * 255;
            noiseTwoData[index + 1] = noiseValue * 255;
            noiseTwoData[index + 2] = noiseValue * 255;
            noiseTwoData[index + 3] = 255;
        }
    }

    // Load sampled data into images, then load into textures to pass into shader
    std::array<sf::Image, 2> waterNoiseImages;

    waterNoiseImages[0].create(waterNoiseSize, waterNoiseSize, noiseData.data());
    waterNoiseImages[1].create(waterNoiseSize, waterNoiseSize, noiseTwoData.data());
    
    waterNoiseTextures[0].loadFromImage(waterNoiseImages[0]);
    waterNoiseTextures[1].loadFromImage(waterNoiseImages[1]);

    // Pass noise textures into water shader
    sf::Shader* waterShader = Shaders::getShader(ShaderType::Water);
    waterShader->setUniform("noise", waterNoiseTextures[0]);
    waterShader->setUniform("noiseTwo", waterNoiseTextures[1]);
}

void Game::updateMusic(float dt)
{
    // Music playing
    if (!Sounds::isMusicFinished())
    {
        return;
    }
    
    musicGapTimer += dt;

    if (musicGapTimer < musicGap)
        return;
    
    // Play new music as music gap has ended
    static constexpr std::array<MusicType, 2> musicTypes = {MusicType::WorldTheme, MusicType::WorldTheme2};
    int musicTypeChance = rand() % musicTypes.size();

    Sounds::playMusic(musicTypes[musicTypeChance], 70.0f);

    musicGapTimer = 0.0f;
    musicGap = MUSIC_GAP_MIN + rand() % 5;
}

void Game::drawMouseCursor()
{
    sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    mouseScreenPos.x = std::max(std::min(mouseScreenPos.x, static_cast<float>(window.getSize().x)), 0.0f);
    mouseScreenPos.y = std::max(std::min(mouseScreenPos.y, static_cast<float>(window.getSize().y)), 0.0f);

    // Switch mouse cursor mode
    sf::IntRect textureRect(80, 32, 8, 8);
    bool shiftMode = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

    if (InventoryGUI::canQuickTransfer(mouseScreenPos, shiftMode, inventory, chestDataPool.getChestDataPtr(openedChestID)))
    {
        textureRect = sf::IntRect(96, 32, 12, 12);
    }

    TextureManager::drawSubTexture(window, {TextureType::UI, mouseScreenPos, 0, {3 * intScale, 3 * intScale}}, textureRect);
}

void Game::drawDebugMenu(float dt)
{
    if (!DebugOptions::debugOptionsMenuOpen)
        return;

    ImGui::Begin("Debug Options", &DebugOptions::debugOptionsMenuOpen);

    // Debug info
    std::vector<std::string> debugStrings = {
        GAME_VERSION,
        std::to_string(static_cast<int>(1.0f / dt)) + "FPS",
        std::to_string(chunkManager.getLoadedChunkCount()) + " Chunks loaded",
        std::to_string(chunkManager.getGeneratedChunkCount()) + " Chunks generated",
        "Seed: " + std::to_string(chunkManager.getSeed()),
        "Player pos: " + std::to_string(static_cast<int>(player.getPosition().x)) + ", " + std::to_string(static_cast<int>(player.getPosition().y))
    };

    for (const std::string& string : debugStrings)
    {
        ImGui::Text(string.c_str());
    }

    ImGui::Spacing();

    int musicVolume = Sounds::getMusicVolume();
    if (ImGui::SliderInt("Music Volume", &musicVolume, 0, 100))
    {
        Sounds::setMusicVolume(musicVolume);
    }

    ImGui::Checkbox("Show Collision Boxes", &DebugOptions::drawCollisionRects);
    ImGui::Checkbox("Show Chunk Boundaries", &DebugOptions::drawChunkBoundaries);
    ImGui::Checkbox("Show Entity Chunk Parents", &DebugOptions::drawEntityChunkParents);

    ImGui::Spacing();

    ImGui::Text("Visible Tiles");

    for (auto iter = DebugOptions::tileMapsVisible.begin(); iter != DebugOptions::tileMapsVisible.end(); iter++)
    {
        ImGui::Checkbox(std::to_string(iter->first).c_str(), &(DebugOptions::tileMapsVisible[iter->first]));
    }

    ImGui::Spacing();

    ImGui::Text("Save / Load");

    if (ImGui::Button("Save"))
    {
        saveGame();
    }

    if (ImGui::Button("Load"))
    {
        loadGame(currentSaveName);
    }

    ImGui::Spacing();

    if (ImGui::Button("Toggle Day / Night"))
    {
        isDay = !isDay;
        if (isDay) worldDarkness = 0.0f;
        else worldDarkness = 0.95f;
    }

    if (ImGui::Checkbox("God Mode", &DebugOptions::godMode))
    {
        DebugOptions::godSpeedMultiplier = 1.0f;
        DebugOptions::limitlessZoom = false;
    }

    if (DebugOptions::godMode)
    {
        ImGui::SliderFloat("God Speed Multiplier", &DebugOptions::godSpeedMultiplier, 0.0f, 100.0f);
        ImGui::Checkbox("Limitless Zoom", &DebugOptions::limitlessZoom);
    }

    ImGui::End();   
}