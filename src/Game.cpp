#include "Game.hpp"

// FIX: Crash on save / rocket enter bug (can't save???)
// FIX: Rockets in rooms

// TODO: Night and menu music
// TODO: Better GUI system / relative to window size etc and texturing
// TODO: Consumable / health regen items

// PRIORITY: HIGH
// TODO: Create event callback system for object responding to triggers, rather than calling game functions directly

// TODO: Structure functionality (item spawns, crafting stations etc)
// TODO: Different types of structures

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
    window.create(sf::VideoMode(videoMode.width, videoMode.height), GAME_TITLE, sf::Style::None);

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
    // TODO: ugly
    Achievements::steamInitialised = steamInitialised;
    if (steamInitialised)
        SteamUserStats()->RequestCurrentStats();

    // Randomise
    srand(time(NULL));

    // Initialise values
    gameTime = 0;
    //mainMenuState = MainMenuState::Main;
    gameState = GameState::MainMenu;
    destinationGameState = gameState;
    transitionGameStateTimer = 0.0f;
    worldMenuState = WorldMenuState::Main;

    mainMenuGUI.initialise();

    //menuScreenshotIndex = 0;
    //menuScreenshotTimer = 0.0f;

    openedChestID = 0xFFFF;

    musicGapTimer = 0.0f;
    musicGap = 0.0f;

    inventory = InventoryData(32);
    armourInventory = InventoryData(3);

    // Initialise day/night cycle
    // dayNightToggleTimer = 0.0f;
    // worldDarkness = 0.0f;
    isDay = true;

    // Initialise GUI
    InventoryGUI::initialise(inventory);

    generateWaterNoiseTexture();

    // Find valid player spawn
    // sf::Vector2f spawnPos = chunkManager.findValidSpawnPosition(2);
    // player.setPosition(spawnPos);

    // Initialise inventory
    giveStartingInventory();

    camera.instantUpdate(player.getPosition());

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

        // runFeatureTest();
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


void Game::runFeatureTest()
{
    // static std::optional<std::pair<int, int>> start, end;
    // static int pathfindTick = 0;

    // const int SCALE = 12;

    // sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    // int mouseTileX = mousePos.x / SCALE;
    // int mouseTileY = mousePos.y / SCALE;

    for (auto event = sf::Event{}; window.pollEvent(event);)
    {
        handleEventsWindow(event);

        if (event.type == sf::Event::KeyPressed)
        {
            // if (event.key.code == sf::Keyboard::R || event.key.code == sf::Keyboard::Tab)
            // {
            //     start = std::nullopt;
            //     end = std::nullopt;
            //     pathfindingEngine.resize(160, 90);
            // }
            // if (event.key.code == sf::Keyboard::P)
            // {
            //     start = {mouseTileX, mouseTileY};
            // }
            // if (event.key.code == sf::Keyboard::L)
            // {
            //     end = {mouseTileX, mouseTileY};
            // }
            // if (event.key.code == sf::Keyboard::Tab)
            // {
            //     for (int x = 0; x < 190; x++)
            //     {
            //         for (int y = 0; y < 90; y++)
            //         {
            //             if (rand() % 4 <= 1)
            //             {
            //                 pathfindingEngine.setObstacle(x, y, true);
            //             }
            //         }
            //     }
            // }
        }
    }

    // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    // {
    //     pathfindingEngine.setObstacle(mouseTileX, mouseTileY, true);
    // }
    // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Backspace))
    // {
    //     pathfindingEngine.setObstacle(mouseTileX, mouseTileY, false);
    // }

    // window.clear();

    // auto& obstacles = pathfindingEngine.getObstacles();
    // for (int i = 0; i < obstacles.size(); i++)
    // {
    //     if (!obstacles[i])
    //     {
    //         continue;
    //     }

    //     sf::RectangleShape rect({SCALE, SCALE});
    //     rect.setPosition(sf::Vector2f(i % 160, std::floor(i / 160)) * static_cast<float>(SCALE));
    //     window.draw(rect);
    // }

    // pathfindTick++;

    // if (start.has_value() && end.has_value())
    // {
    //     static std::vector<PathfindGridCoordinate> pathFound;
    //     if (pathfindTick > 0)
    //     {
    //         pathFound.clear();
    //         pathfindTick = 0;
    //         pathfindingEngine.findPath(start->first, start->second, end->first, end->second, pathFound);
    //     }

    //     for (const auto& coord : pathFound)
    //     {
    //         sf::RectangleShape rect({ SCALE, SCALE });
    //         rect.setPosition(sf::Vector2f(coord.x, coord.y) * static_cast<float>(SCALE));
    //         rect.setFillColor(sf::Color(0, 0, 255));
    //         window.draw(rect);
    //     }
    // }

    // if (start.has_value())
    // {
    //     sf::RectangleShape rect({SCALE, SCALE});
    //     rect.setPosition(sf::Vector2f(start->first, start->second) * static_cast<float>(SCALE));
    //     rect.setFillColor(sf::Color(0, 255, 0));
    //     window.draw(rect);
    // }

    // if (end.has_value())
    // {
    //     sf::RectangleShape rect({SCALE, SCALE});
    //     rect.setPosition(sf::Vector2f(end->first, end->second) * static_cast<float>(SCALE));
    //     rect.setFillColor(sf::Color(255, 0, 0));
    //     window.draw(rect);
    // }
}


// -- Main Menu -- //

void Game::runMainMenu(float dt)
{
    for (auto event = sf::Event{}; window.pollEvent(event);)
    {
        handleEventsWindow(event);

        mainMenuGUI.handleEvent(event);
        // guiContext.processEvent(event);
    }

    sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    mainMenuGUI.update(dt, mouseScreenPos, *this, projectileManager, inventory);

    std::optional<MainMenuEvent> menuEvent = mainMenuGUI.createAndDraw(window, spriteBatch, *this, dt, gameTime);

    spriteBatch.endDrawing(window);

    if (menuEvent.has_value())
    {
        switch (menuEvent->type)
        {
            case MainMenuEventType::StartNew:
            {
                if (isStateTransitioning())
                {
                    break;
                }
                
                mainMenuGUI.setCanInteract(false);

                currentSaveFileSummary = menuEvent->saveFileSummary;
                startNewGame(menuEvent->worldSeed);
                break;
            }
            case MainMenuEventType::Load:
            {
                if (isStateTransitioning())
                {
                    break;
                }

                if (loadGame(menuEvent->saveFileSummary))
                {
                    mainMenuGUI.setCanInteract(false);
                }

                currentSaveFileSummary = menuEvent->saveFileSummary;
                break;
            }
            case MainMenuEventType::Quit:
            {
                window.close();
                ImGui::SFML::Shutdown(window);
                break;
            }
        }
    }
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

        if (ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse)
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

        if (event.type == sf::Event::MouseButtonPressed)
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

    saveSessionPlayTime += dt;

    updateMusic(dt);

    // Update tweens
    floatTween.update(dt);

    // Update particles
    particleSystem.update(dt);

    HitMarkers::update(dt);

    camera.update(player.getPosition(), mouseScreenPos, dt);

    dayCycleManager.update(dt);
    isDay = dayCycleManager.isDay();
    // updateDayNightCycle(dt);

    if (travelPlanetTrigger)
    {
        travelToPlanet(destinationPlanet);
    }

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

    Cursor::setCursorHidden(!player.canReachPosition(Cursor::getMouseWorldPos(window, camera)));

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
                    BuildableObject* rocketObject = getObjectFromChunkOrRoom(rocketEnteredReference);

                    if (rocketObject)
                    {
                        destinationPlanet = selectedDestination;
                        worldMenuState = WorldMenuState::FlyingRocket;
                        rocketObject->triggerBehaviour(*this, ObjectBehaviourTrigger::RocketFlyUp);
                        // Fade out music
                        Sounds::stopMusic(0.5f);
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
        std::vector<std::string> extraInfoStrings;
        if (nearbyCraftingStationLevels.contains(CLOCK_CRAFTING_STATION))
        {
            extraInfoStrings.push_back(dayCycleManager.getDayString());
            extraInfoStrings.push_back(dayCycleManager.getTimeString());
        }

        switch (worldMenuState)
        {
            case WorldMenuState::Main:
                InventoryGUI::drawHotbar(window, mouseScreenPos, inventory);
                InventoryGUI::drawItemPopups(window);
                HealthGUI::drawHealth(window, spriteBatch, player.getHealth(), player.getMaxHealth(), gameTime, extraInfoStrings);
                break;
            
            case WorldMenuState::Inventory:
                InventoryGUI::draw(window, gameTime, mouseScreenPos, inventory, armourInventory, chestDataPool.getChestDataPtr(openedChestID));
                HealthGUI::drawHealth(window, spriteBatch, player.getHealth(), player.getMaxHealth(), gameTime, extraInfoStrings);
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
    Cursor::updateTileCursor(window, camera, dt, chunkManager, player.getCollisionRect(), InventoryGUI::getHeldItemType(inventory), player.getTool());

    // Update player
    bool wrappedAroundWorld = false;
    sf::Vector2f wrapPositionDelta;

    if (!isStateTransitioning())
        player.update(dt, Cursor::getMouseWorldPos(window, camera), chunkManager, worldSize, wrappedAroundWorld, wrapPositionDelta);

    // Handle world wrapping for camera and cursor, if player wrapped around
    if (wrappedAroundWorld)
    {
        camera.handleWorldWrap(wrapPositionDelta);
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
    bool modifiedChunks = chunkManager.updateChunks(*this, camera);
    chunkManager.updateChunksObjects(*this, dt);
    chunkManager.updateChunksEntities(dt, projectileManager, inventory);

    // If modified chunks, force a lighting recalculation
    if (modifiedChunks)
    {
        lightingTick = LIGHTING_TICK;
    }
    
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
}

void Game::drawOnPlanet(float dt)
{
    // Get world objects
    std::vector<WorldObject*> worldObjects = chunkManager.getChunkObjects();
    std::vector<WorldObject*> entities = chunkManager.getChunkEntities();
    worldObjects.insert(worldObjects.end(), entities.begin(), entities.end());
    worldObjects.push_back(&player);
    bossManager.getBossWorldObjects(worldObjects);

    drawWorld(worldTexture, dt, worldObjects, chunkManager, camera);
    drawLighting(dt, worldObjects);

    // UI
    sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    Cursor::drawCursor(window, camera);

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

    HitMarkers::draw(window, camera);

    bossManager.drawStatsAtCursor(window, camera, mouseScreenPos);
}

void Game::drawWorld(sf::RenderTexture& renderTexture, float dt, std::vector<WorldObject*>& worldObjects, ChunkManager& chunkManagerArg, const Camera& cameraArg)
{
    // Draw all world onto texture for lighting
    renderTexture.create(window.getSize().x, window.getSize().y);
    renderTexture.clear();

    // Draw water
    chunkManagerArg.drawChunkWater(renderTexture, cameraArg, gameTime);

    std::sort(worldObjects.begin(), worldObjects.end(), [](WorldObject* a, WorldObject* b)
    {
        if (a->getDrawLayer() != b->getDrawLayer()) return a->getDrawLayer() > b->getDrawLayer();
        if (a->getPosition().y == b->getPosition().y) return a->getPosition().x < b->getPosition().x;
        return a->getPosition().y < b->getPosition().y;
    });

    spriteBatch.beginDrawing();

    // Draw terrain
    chunkManagerArg.drawChunkTerrain(renderTexture, spriteBatch, cameraArg, gameTime);

    // Draw objects
    for (WorldObject* worldObject : worldObjects)
    {
        worldObject->draw(renderTexture, spriteBatch, *this, cameraArg, dt, gameTime, chunkManagerArg.getWorldSize(), {255, 255, 255, 255});
    }

    // Draw projectiles
    projectileManager.drawProjectiles(renderTexture, spriteBatch, cameraArg);

    spriteBatch.endDrawing(renderTexture);

    renderTexture.display();
}

void Game::drawLighting(float dt, std::vector<WorldObject*>& worldObjects)
{
    float lightLevel = dayCycleManager.getLightLevel();

    unsigned char ambientRedLight = Helper::lerp(2, 255, lightLevel);
    unsigned char ambientGreenLight = Helper::lerp(7, 244, lightLevel);
    unsigned char ambientBlueLight = Helper::lerp(14, 234, lightLevel);

    sf::Vector2i chunksSizeInView = chunkManager.getChunksSizeInView(camera);
    sf::Vector2f topLeftChunkPos = chunkManager.topLeftChunkPosInView(camera);
    
    // Draw light sources on light texture
    sf::RenderTexture lightTexture;
    lightTexture.create(chunksSizeInView.x * CHUNK_TILE_SIZE * TILE_LIGHTING_RESOLUTION, chunksSizeInView.y * CHUNK_TILE_SIZE * TILE_LIGHTING_RESOLUTION);

    lightingTick++;
    if (lightingTick >= LIGHTING_TICK)
    {
        lightingTick = 0;

        // Recalculate lighting

        // Prepare lighting engine
        lightingEngine.resize(chunksSizeInView.x * CHUNK_TILE_SIZE * TILE_LIGHTING_RESOLUTION, chunksSizeInView.y * CHUNK_TILE_SIZE * TILE_LIGHTING_RESOLUTION);

        // player.drawLightMask(lightTexture);

        for (WorldObject* worldObject : worldObjects)
        {
            // worldObject->drawLightMask(lightTexture);
            worldObject->createLightSource(lightingEngine, topLeftChunkPos);
        }

        lightingEngine.calculateLighting(sf::Color(255, 220, 140));
    }


    lightTexture.clear({ambientRedLight, ambientGreenLight, ambientBlueLight, 255});

    // draw from lighting engine
    lightingEngine.drawLighting(lightTexture);

    lightTexture.display();
    lightTexture.setSmooth(smoothLighting);

    sf::Sprite lightTextureSprite(lightTexture.getTexture());
    // lightTextureSprite.setColor(sf::Color(255, 255, 255, 255));

    lightTextureSprite.setPosition(camera.worldToScreenTransform(topLeftChunkPos));
    lightTextureSprite.setScale(sf::Vector2f(ResolutionHandler::getScale(), ResolutionHandler::getScale()) * TILE_SIZE_PIXELS_UNSCALED / static_cast<float>(TILE_LIGHTING_RESOLUTION));

    worldTexture.draw(lightTextureSprite, sf::BlendMultiply);

    worldTexture.display();

    sf::Sprite worldTextureSprite(worldTexture.getTexture());
    window.draw(worldTextureSprite);
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
        const StructureData& structureData = StructureDataLoader::getStructureData(enterEvent.enteredStructure->getStructureType());
        structureEnteredID = structureRoomPool.createRoom(structureData.roomType, chestDataPool);
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
    switch (gameState)
    {
        case GameState::OnPlanet:
        {
            rocketEnteredReference.chunk = rocket.getChunkInside(chunkManager.getWorldSize());
            rocketEnteredReference.tile = rocket.getChunkTileInside(chunkManager.getWorldSize());
            break;
        }
        case GameState::InStructure:
        {
            rocketEnteredReference.chunk = ChunkPosition(0, 0);
            rocketEnteredReference.tile = rocket.getTileInside();
            break;
        }
    }

    // Save just before enter
    saveGame(true);

    worldMenuState = WorldMenuState::TravelSelect;

    TravelSelectGUI::setAvailableDestinations(rocket.getRocketAvailableDestinations(chunkManager.getPlanetType()));

    player.enterRocket(rocket.getRocketPosition());
}

void Game::exitRocket()
{
    worldMenuState = WorldMenuState::Main;

    // Get rocket object
    BuildableObject* rocketObject = getObjectFromChunkOrRoom(rocketEnteredReference);

    // Exit rocket object
    if (rocketObject)
    {
        rocketObject->triggerBehaviour(*this, ObjectBehaviourTrigger::RocketExit);
    }
    else
    {
        std::cout << "Error: Attempted to exit null rocket object at ";
        std::cout << rocketEnteredReference.chunk.x << ", " << rocketEnteredReference.chunk.y << ":";
        std::cout << rocketEnteredReference.tile.x << ", " << rocketEnteredReference.tile.y << "\n";
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

    Cursor::updateTileCursorInRoom(window, camera, dt, structureRoom, InventoryGUI::getHeldItemType(inventory), player.getTool());

    if (!isStateTransitioning())
        player.updateInStructure(dt, Cursor::getMouseWorldPos(window, camera), structureRoom);

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
    structureRoom.draw(window, camera);

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
        object->draw(window, spriteBatch, *this, camera, dt, gameTime, chunkManager.getWorldSize(), sf::Color(255, 255, 255));
    }

    spriteBatch.endDrawing(window);

    Cursor::drawCursor(window, camera);
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
    sf::Vector2f mouseWorldPos = Cursor::getMouseWorldPos(window, camera);
    
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

    sf::Vector2f mouseWorldPos = Cursor::getMouseWorldPos(window, camera);

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
    sf::Vector2f mouseWorldPos = Cursor::getMouseWorldPos(window, camera);

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
    sf::Vector2f mouseWorldPos = Cursor::getMouseWorldPos(window, camera);

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

    bool inRange = player.canReachPosition(Cursor::getMouseWorldPos(window, camera));

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
        chunkManager.setObject(Cursor::getSelectedChunk(chunkManager.getWorldSize()), Cursor::getSelectedChunkTile(), objectType, *this);
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
    
    if (!player.canReachPosition(Cursor::getMouseWorldPos(window, camera)))
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
    bossManager.createBoss(itemData.bossSummonData->bossName, player.getPosition(), *this);
}

void Game::drawGhostPlaceObjectAtCursor(ObjectType object)
{
    // Draw object to be placed if held
    bool canPlace = chunkManager.canPlaceObject(Cursor::getSelectedChunk(chunkManager.getWorldSize()),
                                                Cursor::getSelectedChunkTile(),
                                                object,
                                                player.getCollisionRect());

    bool inRange = player.canReachPosition(Cursor::getMouseWorldPos(window, camera));

    sf::Color drawColor(255, 0, 0, 180);
    if (canPlace && inRange)
        drawColor = sf::Color(0, 255, 0, 180);
    
    BuildableObject objectGhost(Cursor::getLerpedSelectPos() + sf::Vector2f(TILE_SIZE_PIXELS_UNSCALED / 2.0f, TILE_SIZE_PIXELS_UNSCALED / 2.0f), object, false);

    objectGhost.draw(window, spriteBatch, *this, camera, 0.0f, 0, chunkManager.getWorldSize(), drawColor);

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
        .position = camera.worldToScreenTransform(tileWorldPosition),
        .scale = {scale, scale},
        .colour = landGhostColor
    }, textureRect);
}

BuildableObject* Game::getSelectedObjectFromChunkOrRoom()
{
    sf::Vector2f mouseWorldPos = Cursor::getMouseWorldPos(window, camera);

    if (gameState == GameState::OnPlanet)
    {
        BuildableObject* selectedObject = chunkManager.getChunkObject(
            Cursor::getSelectedChunk(chunkManager.getWorldSize()), Cursor::getSelectedChunkTile());
        
        return selectedObject;
    }
    else if (gameState == GameState::InStructure)
    {
        Room& structureRoom = structureRoomPool.getRoom(structureEnteredID);
        
        return structureRoom.getObject(Cursor::getSelectedTile());
    }

    return nullptr;
}

BuildableObject* Game::getObjectFromChunkOrRoom(ObjectReference objectReference)
{
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

    // TODO: Set state to either on planet or in structure based on planet load data, rather than default to on planet
    // overrideState(GameState::OnPlanet);

    travelPlanetTrigger = false;

    player.exitRocket();

    resetChestDataPool();
    resetStructureRoomPool();
    bossManager.clearBosses();

    if (!loadPlanet(planetType))
    {
        overrideState(GameState::OnPlanet);
        initialiseNewPlanet(planetType, true);
    }

    camera.instantUpdate(player.getPosition());

    if (gameState == GameState::OnPlanet)
    {
        chunkManager.updateChunks(*this, camera);
    }
    lightingTick = LIGHTING_TICK;

    // Start rocket flying downwards
    // BuildableObject* rocketObject = chunkManager.getChunkObject(rocketEnteredReference.chunk, rocketEnteredReference.tile);
    BuildableObject* rocketObject = getObjectFromChunkOrRoom(rocketEnteredReference);
    if (rocketObject)
    {
        rocketObject->triggerBehaviour(*this, ObjectBehaviourTrigger::RocketFlyDown);
    }
    // RocketObject* rocket = static_cast<RocketObject*>(chunkManager.getChunkObject(rocketObject.chunk, rocketObject.tile));

    // player.enterRocket(rocket->getRocketPosition());
    // startFlyingRocket(chunkManager.getPlanetType(), true);

    camera.instantUpdate(player.getPosition());
}

void Game::initialiseNewPlanet(PlanetType planetType, bool placeRocket)
{
    chunkManager.setPlanetType(planetType);

    ChunkPosition playerSpawnChunk = chunkManager.findValidSpawnChunk(2);

    sf::Vector2f playerSpawnPos;
    playerSpawnPos.x = playerSpawnChunk.x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    playerSpawnPos.y = playerSpawnChunk.y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    player.setPosition(playerSpawnPos);
    
    camera.instantUpdate(player.getPosition());

    chunkManager.updateChunks(*this, camera);

    // Ensure spawn chunk does not have structure
    chunkManager.regenerateChunkWithoutStructure(playerSpawnChunk, *this);
    
    // Place rocket
    if (placeRocket)
    {
        chunkManager.setObject(playerSpawnChunk, sf::Vector2i(0, 0), ObjectDataLoader::getObjectTypeFromName("Rocket Launch Pad"), *this);
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
            
            if (gameState == GameState::OnPlanet)
            {
                sf::Vector2f roomEntrancePos = structureRoomPool.getRoom(structureEnteredID).getEntrancePosition();

                player.setPosition(roomEntrancePos);
            }

            camera.instantUpdate(player.getPosition());

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
                camera.instantUpdate(player.getPosition());
            }
            break;
        }
    }

    gameState = newState;
}

void Game::overrideState(GameState newState)
{
    gameState = newState;
    destinationGameState = newState;
}


// -- Save / load -- //

void Game::startNewGame(int seed)
{
    // setWorldSeedFromInput();

    chunkManager.setSeed(seed);

    initialiseNewPlanet(PlanetGenDataLoader::getPlanetTypeFromName("Earthlike"));

    dayCycleManager.setCurrentTime(dayCycleManager.getDayLength() * 0.5f);
    dayCycleManager.setCurrentDay(1);

    saveSessionPlayTime = 0.0f;

    bossManager.clearBosses();

    camera.instantUpdate(player.getPosition());

    chunkManager.updateChunks(*this, camera);
    lightingTick = LIGHTING_TICK;

    startChangeStateTransition(GameState::OnPlanet);
}

bool Game::saveGame(bool gettingInRocket)
{
    GameSaveIO io(currentSaveFileSummary.name);

    PlayerGameSave playerGameSave;
    playerGameSave.seed = chunkManager.getSeed();
    playerGameSave.planetType = chunkManager.getPlanetType();
    // playerGameSave.playerPos = player.getPosition();
    playerGameSave.inventory = inventory;
    playerGameSave.armourInventory = armourInventory;
    playerGameSave.time = dayCycleManager.getCurrentTime();
    playerGameSave.day = dayCycleManager.getCurrentDay();

    // Add play time
    currentSaveFileSummary.timePlayed += std::round(saveSessionPlayTime);
    saveSessionPlayTime = 0.0f;
    playerGameSave.timePlayed = currentSaveFileSummary.timePlayed;

    PlanetGameSave planetGameSave;
    planetGameSave.chunks = chunkManager.getChunkPODs();
    planetGameSave.chestDataPool = chestDataPool;
    planetGameSave.structureRoomPool = structureRoomPool;

    if (gameState == GameState::OnPlanet)
    {
        planetGameSave.playerLastPlanetPos = player.getPosition();
    }
    else if (gameState == GameState::InStructure)
    {
        planetGameSave.isInRoom = true;
        planetGameSave.inRoomID = structureEnteredID;
        planetGameSave.positionInRoom = player.getPosition();

        planetGameSave.playerLastPlanetPos = structureEnteredPos;
    }
    
    if (gettingInRocket)
    {
        planetGameSave.rocketObjectUsed = rocketEnteredReference;
    }

    io.write(playerGameSave, planetGameSave);

    return true;
}

bool Game::loadGame(const SaveFileSummary& saveFileSummary)
{
    GameSaveIO io(saveFileSummary.name);

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
    dayCycleManager.setCurrentTime(playerGameSave.time);
    dayCycleManager.setCurrentDay(playerGameSave.day);

    changePlayerTool();

    chunkManager.loadFromChunkPODs(planetGameSave.chunks, *this);
    chestDataPool = planetGameSave.chestDataPool;
    structureRoomPool = planetGameSave.structureRoomPool;

    GameState nextGameState = GameState::OnPlanet;

    if (planetGameSave.isInRoom)
    {
        player.setPosition(planetGameSave.positionInRoom);
        structureEnteredID = planetGameSave.inRoomID;
        structureEnteredPos = planetGameSave.playerLastPlanetPos;

        nextGameState = GameState::InStructure;
    }
    else
    {
        player.setPosition(planetGameSave.playerLastPlanetPos);
    }

    bossManager.clearBosses();

    camera.instantUpdate(player.getPosition());

    chunkManager.updateChunks(*this, camera);
    lightingTick = LIGHTING_TICK;

    // Load successful, set save name as current save and start state transition
    currentSaveFileSummary = saveFileSummary;
    startChangeStateTransition(nextGameState);

    saveSessionPlayTime = 0.0f;

    // Fade out previous music
    Sounds::stopMusic(0.3f);

    return true;
}

bool Game::loadPlanet(PlanetType planetType)
{
    GameSaveIO io(currentSaveFileSummary.name);

    PlanetGameSave planetGameSave;

    if (!io.loadPlanet(planetType, planetGameSave))
    {
        return false;
    }

    chunkManager.setPlanetType(planetType);

    if (planetGameSave.isInRoom)
    {
        overrideState(GameState::InStructure);
        player.setPosition(planetGameSave.positionInRoom);
        structureEnteredID = planetGameSave.inRoomID;
        structureEnteredPos = planetGameSave.playerLastPlanetPos;
    }
    else
    {
        overrideState(GameState::OnPlanet);
        player.setPosition(planetGameSave.playerLastPlanetPos);
    }

    chunkManager.loadFromChunkPODs(planetGameSave.chunks, *this);
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

    camera.handleScaleChange(beforeScale, afterScale, player.getPosition());
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
    
    window.create(videoMode, GAME_TITLE, windowStyle);

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

    camera.instantUpdate(player.getPosition());

    // float afterScale = ResolutionHandler::getScale();

    // if (beforeScale != afterScale)
        // camera.handleScaleChange(beforeScale, afterScale, player.getPosition());
}


// -- Misc -- //

const DayCycleManager& Game::getDayCycleManager()
{
    if (gameState == GameState::MainMenu)
    {
        // Return a new day cycle manager while in main menu
        // Prevents world in background changing when a save is loaded
        static DayCycleManager tempDayCycleManager;
        return tempDayCycleManager;
    }

    return dayCycleManager;
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
        loadGame(currentSaveFileSummary);
    }

    ImGui::Spacing();

    ImGui::Checkbox("Smooth Lighting", &smoothLighting);

    float time = dayCycleManager.getCurrentTime();
    if (ImGui::SliderFloat("Day time", &time, 0.0f, dayCycleManager.getDayLength()))
    {
        dayCycleManager.setCurrentTime(time);
    }

    ImGui::Text(("Light level: " + std::to_string(dayCycleManager.getLightLevel())).c_str());

    ImGui::Spacing();

    static char* itemToGive = new char[100];
    ImGui::InputText("Give item", itemToGive, 100);
    if (ImGui::Button("Give Item"))
    {
        inventory.addItem(ItemDataLoader::getItemTypeFromName(itemToGive), 1);
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