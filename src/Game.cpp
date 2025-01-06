#include "Game.hpp"

// TODO: Controller support for using tools + building
// TODO: Controller glyphs

// FIX: Improve UI scaling elements (text etc)

// TODO: Night and menu music

// PRIORITY: LOW
// TODO: Fishing rod draw order (split into separate objects?)



// -- Public methods / entry point -- //

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
    if(!ToolDataLoader::loadData("Data/Info/tools.data")) return false;
    if(!ArmourDataLoader::loadData("Data/Info/armour.data")) return false;
    if(!EntityDataLoader::loadData("Data/Info/entities.data")) return false;
    if(!ObjectDataLoader::loadData("Data/Info/objects.data")) return false;
    if(!RecipeDataLoader::loadData("Data/Info/item_recipes.data")) return false;
    if(!StructureDataLoader::loadData("Data/Info/structures.data")) return false;
    if(!PlanetGenDataLoader::loadData("Data/Info/planet_generation.data")) return false;

    // Must be done once all other data is loaded to avoid circular dependency
    ObjectDataLoader::loadRocketPlanetDestinations(PlanetGenDataLoader::getPlanetStringToTypeMap(), StructureDataLoader::getRoomTravelLocationNameToTypeMap());

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

    loadOptions();

    // Create key bindings
    InputManager::bindKey(InputAction::WALK_UP, sf::Keyboard::W);
    InputManager::bindKey(InputAction::WALK_DOWN, sf::Keyboard::S);
    InputManager::bindKey(InputAction::WALK_LEFT, sf::Keyboard::A);
    InputManager::bindKey(InputAction::WALK_RIGHT, sf::Keyboard::D);
    InputManager::bindKey(InputAction::OPEN_INVENTORY, sf::Keyboard::E);
    InputManager::bindKey(InputAction::UI_BACK, sf::Keyboard::Escape);
    InputManager::bindKey(InputAction::PAUSE_GAME, sf::Keyboard::Escape);
    InputManager::bindMouseButton(InputAction::USE_TOOL, sf::Mouse::Button::Left);
    InputManager::bindMouseButton(InputAction::INTERACT, sf::Mouse::Button::Right);
    InputManager::bindMouseWheel(InputAction::ZOOM_IN, MouseWheelScroll::Up);
    InputManager::bindMouseWheel(InputAction::ZOOM_OUT, MouseWheelScroll::Down);
    InputManager::bindMouseWheel(InputAction::UI_TAB_LEFT, MouseWheelScroll::Down);
    InputManager::bindMouseWheel(InputAction::UI_TAB_RIGHT, MouseWheelScroll::Up);

    InputManager::bindControllerAxis(InputAction::WALK_UP, JoystickAxisWithDirection{sf::Joystick::Axis::Y, JoystickAxisDirection::NEGATIVE});
    InputManager::bindControllerAxis(InputAction::WALK_DOWN, JoystickAxisWithDirection{sf::Joystick::Axis::Y, JoystickAxisDirection::POSITIVE});
    InputManager::bindControllerAxis(InputAction::WALK_LEFT, JoystickAxisWithDirection{sf::Joystick::Axis::X, JoystickAxisDirection::NEGATIVE});
    InputManager::bindControllerAxis(InputAction::WALK_RIGHT, JoystickAxisWithDirection{sf::Joystick::Axis::X, JoystickAxisDirection::POSITIVE});

    InputManager::bindControllerButton(InputAction::OPEN_INVENTORY, 1);
    InputManager::bindControllerButton(InputAction::UI_CONFIRM, 0);
    InputManager::bindControllerButton(InputAction::UI_CONFIRM_OTHER, 2);
    InputManager::bindControllerButton(InputAction::UI_BACK, 1);
    InputManager::bindControllerButton(InputAction::PAUSE_GAME, 7);
    InputManager::bindControllerAxis(InputAction::USE_TOOL, JoystickAxisWithDirection{sf::Joystick::Axis::Z, JoystickAxisDirection::NEGATIVE});
    InputManager::bindControllerAxis(InputAction::INTERACT, JoystickAxisWithDirection{sf::Joystick::Axis::Z, JoystickAxisDirection::POSITIVE});
    InputManager::bindControllerAxis(InputAction::UI_UP, JoystickAxisWithDirection{sf::Joystick::Axis::PovY, JoystickAxisDirection::POSITIVE});
    InputManager::bindControllerAxis(InputAction::UI_DOWN, JoystickAxisWithDirection{sf::Joystick::Axis::PovY, JoystickAxisDirection::NEGATIVE});
    InputManager::bindControllerAxis(InputAction::UI_LEFT, JoystickAxisWithDirection{sf::Joystick::Axis::PovX, JoystickAxisDirection::NEGATIVE});
    InputManager::bindControllerAxis(InputAction::UI_RIGHT, JoystickAxisWithDirection{sf::Joystick::Axis::PovX, JoystickAxisDirection::POSITIVE});
    InputManager::bindControllerButton(InputAction::ZOOM_IN, 4);
    InputManager::bindControllerButton(InputAction::ZOOM_OUT, 5);
    InputManager::bindControllerButton(InputAction::UI_TAB_LEFT, 4);
    InputManager::bindControllerButton(InputAction::UI_TAB_RIGHT, 5);

    InputManager::setControllerAxisDeadzone(0.3f);

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

    player = Player(sf::Vector2f(0, 0), &armourInventory);
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
    // giveStartingInventory();

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
        
        InputManager::update();

        window.setView(view);

        // runFeatureTest();
        switch (gameState)
        {
            case GameState::MainMenu:
                runMainMenu(dt);
                break;
            
            case GameState::OnPlanet: // fallthrough
            case GameState::InStructure: // fallthrough
            case GameState::InRoomDestination:
                runInGame(dt);
                break;
        }

        if (isStateTransitioning())
        {
            updateStateTransition(dt);
            drawStateTransition();
        }

        if (!InputManager::isControllerActive())
        {
            drawMouseCursor();
        }

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
    }

    sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    mainMenuGUI.update(dt, mouseScreenPos, *this, projectileManager, inventory);

    window.clear();

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
            case MainMenuEventType::SaveOptions:
            {
                saveOptions();
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

// -- Main Game -- //

void Game::runInGame(float dt)
{
    sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    bool shiftMode = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

    // Handle events
    for (auto event = sf::Event{}; window.pollEvent(event);)
    {
        handleEventsWindow(event);

        // Always process events even when GUI is not drawn
        // Prevents previous state being retained
        travelSelectGUI.handleEvent(event);
        landmarkSetGUI.handleEvent(event);
        npcInteractionGUI.handleEvent(event);
        mainMenuGUI.handleEvent(event);
    }

    // Input testing
    if (!isStateTransitioning() && player.isAlive() && !(ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse))
    {
        if (InputManager::isActionJustActivated(InputAction::USE_TOOL))
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
                        attemptUseConsumable();
                    }
                    else
                    {
                        changePlayerTool();
                    }
                    break;
                }
                case WorldMenuState::NPCShop: // fallthrough
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
                        attemptUseConsumable();
                    }

                    if (itemHeldBefore != InventoryGUI::getHeldItemType(inventory))
                    {
                        changePlayerTool();
                    }
                    break;
                }
            }
        }

        if (InputManager::isActionJustActivated(InputAction::INTERACT))
        {
            switch (worldMenuState)
            {
                case WorldMenuState::Main:
                    attemptObjectInteract();
                    break;
                case WorldMenuState::NPCShop: // fallthrough
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

        if (float zoom = InputManager::getActionAxisImmediateActivation(InputAction::ZOOM_IN, InputAction::ZOOM_OUT);
            std::abs(zoom) >= 0.5f)
        {
            if ((worldMenuState == WorldMenuState::Inventory || worldMenuState == WorldMenuState::NPCShop) &&
                (!InventoryGUI::isMouseOverUI(mouseScreenPos) || InputManager::isControllerActive()))
            {
                handleZoom(zoom);
            }
        }

        if (float tabDelta = InputManager::getActionAxisImmediateActivation(InputAction::UI_TAB_LEFT, InputAction::UI_TAB_RIGHT);
            std::abs(tabDelta) >= 0.5f)
        {
            switch (worldMenuState)
            {
                case WorldMenuState::NPCShop: // fallthrough
                case WorldMenuState::Inventory:
                    if (!InputManager::isControllerActive())
                    {
                        InventoryGUI::handleScroll(mouseScreenPos, tabDelta, inventory);
                    }
                    break;
                case WorldMenuState::Main:
                    InventoryGUI::handleScrollHotbar(tabDelta);
                    changePlayerTool();
                    break;
            }
        }

        if (InputManager::isActionJustActivated(InputAction::OPEN_INVENTORY))
        {
            switch (worldMenuState)
            {
                case WorldMenuState::Main:
                {
                    worldMenuState = WorldMenuState::Inventory;
                    closeChest();
                    InputManager::consumeInputAction(InputAction::OPEN_INVENTORY);
                    break;
                }
                case WorldMenuState::NPCShop: // fallthrough
                case WorldMenuState::Inventory:
                {
                    handleInventoryClose();
                    player.setCanMove(true);
                    InputManager::consumeInputAction(InputAction::OPEN_INVENTORY);
                    break;
                }
            }
        }
        
        if (InputManager::isActionJustActivated(InputAction::PAUSE_GAME))
        {
            if (worldMenuState == WorldMenuState::Main)
            {
                worldMenuState = WorldMenuState::PauseMenu;
                mainMenuGUI.resetHoverRect();
                InputManager::consumeInputAction(InputAction::PAUSE_GAME);
            }
        }
        
        if (InputManager::isActionJustActivated(InputAction::UI_BACK))
        {
            switch (worldMenuState)
            {
                case WorldMenuState::SettingLandmark: // fallthrough
                case WorldMenuState::PauseMenu:
                {
                    worldMenuState = WorldMenuState::Main;
                    player.setCanMove(true);
                    InputManager::consumeInputAction(InputAction::UI_BACK);
                    break;
                }
                case WorldMenuState::NPCShop:
                {
                    InventoryGUI::shopClosed(); // fallthrough
                }
                case WorldMenuState::Inventory:
                {
                    handleInventoryClose();
                    player.setCanMove(true);
                    InputManager::consumeInputAction(InputAction::UI_BACK);
                    break;
                }
                case WorldMenuState::TravelSelect:
                {
                    exitRocket();
                    InputManager::consumeInputAction(InputAction::UI_BACK);
                    break;
                }
                case WorldMenuState::NPCInteract:
                {
                    npcInteractionGUI.close();
                    worldMenuState = WorldMenuState::Main;
                    player.setCanMove(true);
                    InputManager::consumeInputAction(InputAction::UI_BACK);
                    break;
                }
            }
        }
    }

    //
    // -- UPDATING --
    //

    if (worldMenuState != WorldMenuState::PauseMenu)
    {
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

        if (travelTrigger)
        {
            travelToDestination();
        }

        // Update depending on game state
        switch (gameState)
        {
            case GameState::OnPlanet:
                updateOnPlanet(dt);
                break;
            case GameState::InStructure:
            {
                Room& structureRoom = structureRoomPool.getRoom(structureEnteredID);
                updateInRoom(dt, structureRoom, true);
                break;
            }
            case GameState::InRoomDestination:
            {
                updateInRoom(dt, roomDestination, false);
                break;
            }
        }

        Cursor::setCursorHidden(!player.canReachPosition(Cursor::getMouseWorldPos(window, camera)));
        Cursor::setCursorHidden((worldMenuState != WorldMenuState::Main && worldMenuState != WorldMenuState::Inventory) ||
                                !player.isAlive());

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
                case WorldMenuState::NPCShop: // fallthrough
                case WorldMenuState::Inventory:
                {
                    // Update inventory GUI available recipes if required, and animations
                    InventoryGUI::updateAvailableRecipes(inventory, nearbyCraftingStationLevels);
                    InventoryGUI::updateInventory(mouseScreenPos, dt, inventory, armourInventory, chestDataPool.getChestDataPtr(openedChestID));
                    break;
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
        {
            drawOnPlanet(dt);
            break;
        }
        case GameState::InStructure:
        {
            const Room& structureRoom = structureRoomPool.getRoom(structureEnteredID);
            drawInRoom(dt, structureRoom);
            break;
        }
        case GameState::InRoomDestination:
        {
            drawInRoom(dt, roomDestination);
            break;
        }
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
                HealthGUI::drawHealth(window, spriteBatch, player, gameTime, extraInfoStrings);
                break;
            
            case WorldMenuState::Inventory:
                InventoryGUI::handleControllerInput(inventory, armourInventory, chestDataPool.getChestDataPtr(openedChestID));
                HealthGUI::drawHealth(window, spriteBatch, player, gameTime, extraInfoStrings);
                spriteBatch.endDrawing(window);
                InventoryGUI::drawItemPopups(window);
                InventoryGUI::draw(window, gameTime, mouseScreenPos, inventory, armourInventory, chestDataPool.getChestDataPtr(openedChestID));
                break;
            
            
            case WorldMenuState::TravelSelect:
            {
                // std::vector<PlanetType> availableDestinations = getRocketAvailableDestinations();
                PlanetType selectedPlanetDestination = -1;
                RoomType selectedRoomDestination = -1;

                if (travelSelectGUI.createAndDraw(window, dt, selectedPlanetDestination, selectedRoomDestination))
                {
                    BuildableObject* rocketObject = getObjectFromChunkOrRoom(rocketEnteredReference);

                    if (rocketObject)
                    {
                        destinationPlanet = selectedPlanetDestination;
                        destinationRoom = selectedRoomDestination;
                        worldMenuState = WorldMenuState::FlyingRocket;
                        rocketObject->triggerBehaviour(*this, ObjectBehaviourTrigger::RocketFlyUp);
                        // Fade out music
                        Sounds::stopMusic(0.5f);
                        musicGap = MUSIC_GAP_MIN;
                    }
                }
                break;
            }

            case WorldMenuState::SettingLandmark:
            {
                LandmarkSetGUIEvent landmarkSetGUIEvent = landmarkSetGUI.createAndDraw(window, dt);

                if (landmarkSetGUIEvent.modified)
                {
                    BuildableObject* objectPtr = getObjectFromChunkOrRoom(landmarkSetGUI.getLandmarkObjectReference());
                    if (LandmarkObject* landmarkObjectPtr = dynamic_cast<LandmarkObject*>(objectPtr); landmarkObjectPtr != nullptr)
                    {
                        landmarkObjectPtr->setLandmarkColour(landmarkSetGUI.getColourA(), landmarkSetGUI.getColourB());
                    }
                }
                if (landmarkSetGUIEvent.closed)
                {
                    player.setCanMove(true);
                    worldMenuState = WorldMenuState::Main;
                }

                break;
            }

            case WorldMenuState::NPCInteract:
            {
                std::optional<NPCInteractionGUIEvent> npcInteractionGUIEvent = npcInteractionGUI.createAndDraw(window, spriteBatch, dt, gameTime);

                if (npcInteractionGUIEvent.has_value())
                {
                    switch (npcInteractionGUIEvent->type)
                    {
                        case NPCInteractionGUIEventType::Shop:
                        {
                            InventoryGUI::shopOpened(npcInteractionGUIEvent->shopInventoryData);
                            worldMenuState = WorldMenuState::NPCShop;
                            break;
                        }
                        case NPCInteractionGUIEventType::Exit:
                        {
                            npcInteractionGUI.close();
                            player.setCanMove(true);
                            worldMenuState = WorldMenuState::Main;
                            break;
                        }
                    }
                }
                break;
            }
            case WorldMenuState::NPCShop:
            {
                InventoryGUI::draw(window, gameTime, mouseScreenPos, inventory, armourInventory, nullptr);
                break;
            }
        }
    }
    else
    {
        HealthGUI::drawDeadPrompt(window);
    }

    if (worldMenuState == WorldMenuState::PauseMenu)
    {
        std::optional<PauseMenuEventType> pauseMenuEvent = mainMenuGUI.createAndDrawPauseMenu(window, dt, gameTime);

        if (pauseMenuEvent.has_value())
        {
            switch (pauseMenuEvent.value())
            {
                case PauseMenuEventType::Resume:
                {
                    worldMenuState = WorldMenuState::Main;
                    break;
                }
                case PauseMenuEventType::Quit:
                {
                    saveGame();
                    currentSaveFileSummary.name = "";
                    startChangeStateTransition(GameState::MainMenu);
                    mainMenuGUI.initialise();
                    Sounds::stopMusic();
                    break;
                }
            }
        }
    }

    spriteBatch.endDrawing(window);
}


// -- On Planet -- //

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
    {
        player.update(dt, Cursor::getMouseWorldPos(window, camera), chunkManager, enemyProjectileManager, wrappedAroundWorld, wrapPositionDelta);
    }

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
        enemyProjectileManager.handleWorldWrap(wrapPositionDelta);

        // Wrap hit markers
        HitMarkers::handleWorldWrap(wrapPositionDelta);

        // Wrap particles
        particleSystem.handleWorldWrap(wrapPositionDelta);
    }

    // Update (loaded) chunks
    bool modifiedChunks = chunkManager.updateChunks(*this, camera);
    chunkManager.updateChunksObjects(*this, dt);
    chunkManager.updateChunksEntities(dt, projectileManager, inventory, *this);

    // If modified chunks, force a lighting recalculation
    if (modifiedChunks)
    {
        lightingTick = LIGHTING_TICK;
    }
    
    // Get nearby crafting stations
    nearbyCraftingStationLevels = chunkManager.getNearbyCraftingStationLevels(player.getChunkInside(worldSize), player.getChunkTileInside(worldSize), 4);

    // Update bosses
    bossManager.update(*this, projectileManager, enemyProjectileManager, inventory, player, dt);

    // Update projectiles
    projectileManager.update(dt);
    enemyProjectileManager.update(dt);

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

    if (player.getTool() < 0 && (worldMenuState == WorldMenuState::Main || worldMenuState == WorldMenuState::Inventory))
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

    drawLandmarks();

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

    // Draw particles
    particleSystem.draw(renderTexture, spriteBatch, cameraArg);

    // Draw objects
    for (WorldObject* worldObject : worldObjects)
    {
        worldObject->draw(renderTexture, spriteBatch, *this, cameraArg, dt, gameTime, chunkManagerArg.getWorldSize(), {255, 255, 255, 255});
    }

    // Draw projectiles
    projectileManager.drawProjectiles(renderTexture, spriteBatch, cameraArg);
    enemyProjectileManager.drawProjectiles(renderTexture, spriteBatch, cameraArg);

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

void Game::drawLandmarks()
{
    AnimatedTexture landmarkUIAnimation(6, 16, 16, 96, 112, 0.1);

    landmarkUIAnimation.setFrame(static_cast<int>(gameTime / 0.1) % 6);

    for (const LandmarkSummaryData& landmarkSummary : landmarkManager.getLandmarkSummaryDatas(player, chunkManager))
    {
        if (camera.isInView(landmarkSummary.worldPos))
        {
            continue;
        }

        static const int PADDING = 0;
        float intScale = ResolutionHandler::getResolutionIntegerScale();
        sf::Vector2u resolution = ResolutionHandler::getResolution();

        sf::Vector2f screenPos = camera.worldToScreenTransform(landmarkSummary.worldPos);
        screenPos.x = std::clamp(screenPos.x, PADDING * intScale, resolution.x - PADDING * intScale);
        screenPos.y = std::clamp(screenPos.y, PADDING * intScale, resolution.y - PADDING * intScale);

        TextureDrawData drawData;
        drawData.type = TextureType::UI;
        drawData.position = screenPos;
        drawData.scale = sf::Vector2f(3, 3) * intScale;
        drawData.centerRatio = sf::Vector2f(0.5f, 0.5f);
        drawData.colour = sf::Color(255, 255, 255, 150);

        sf::Glsl::Vec4 replaceKeys[2] = {sf::Glsl::Vec4(sf::Color(255, 255, 255)), sf::Glsl::Vec4(sf::Color(0, 0, 0))};
        sf::Glsl::Vec4 replaceValues[2] = {sf::Glsl::Vec4(landmarkSummary.colourA), sf::Glsl::Vec4(landmarkSummary.colourB)};

        sf::Shader* replaceColourShader = Shaders::getShader(ShaderType::ReplaceColour);
        replaceColourShader->setUniform("replaceKeyCount", 2);
        replaceColourShader->setUniformArray("replaceKeys", replaceKeys, 2);
        replaceColourShader->setUniformArray("replaceValues", replaceValues, 2);

        TextureManager::drawSubTexture(window, drawData, landmarkUIAnimation.getTextureRect(), replaceColourShader);
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
        case GameState::InStructure: // fallthrough
        case GameState::InRoomDestination:
        {
            rocketEnteredReference.chunk = ChunkPosition(0, 0);
            rocketEnteredReference.tile = rocket.getTileInside();
            break;
        }
    }

    handleInventoryClose();
    
    // Save just before enter
    saveGame(true);

    worldMenuState = WorldMenuState::TravelSelect;

    PlanetType currentPlanetType = -1;
    RoomType currentRoomType = -1;
    if (gameState == GameState::OnPlanet)
    {
        currentPlanetType = chunkManager.getPlanetType();
    }
    else if (gameState == GameState::InRoomDestination)
    {
        currentRoomType = roomDestination.getRoomType();
    }

    std::vector<PlanetType> planetDestinations;
    std::vector<RoomType> roomDestinations;

    rocket.getRocketAvailableDestinations(currentPlanetType, currentRoomType, planetDestinations, roomDestinations);

    travelSelectGUI.setAvailableDestinations(planetDestinations, roomDestinations);

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
    travelTrigger = true;
}

void Game::rocketFinishedDown(RocketObject& rocket)
{
    exitRocket();
}

// NPC
void Game::interactWithNPC(NPCObject& npc)
{
    handleInventoryClose();
    player.setCanMove(false);

    npcInteractionGUI.initialise(npc);
    worldMenuState = WorldMenuState::NPCInteract;
}

// Landmark
void Game::landmarkPlaced(const LandmarkObject& landmark, bool createGUI)
{
    ObjectReference landmarkObjectReference = ObjectReference{landmark.getChunkInside(chunkManager.getWorldSize()), landmark.getChunkTileInside(chunkManager.getWorldSize())};

    landmarkManager.addLandmark(landmarkObjectReference);

    if (createGUI)
    {
        handleInventoryClose();
        worldMenuState = WorldMenuState::SettingLandmark;
        landmarkSetGUI.initialise(landmarkObjectReference, landmark.getColourA(), landmark.getColourB());

        player.setCanMove(false);
    }
}

void Game::landmarkDestroyed(const LandmarkObject& landmark)
{
    landmarkManager.removeLandmark(ObjectReference{landmark.getChunkInside(chunkManager.getWorldSize()), landmark.getChunkTileInside(chunkManager.getWorldSize())});
}

// -- In Room -- //

void Game::updateInRoom(float dt, Room& room, bool inStructure)
{
    sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    // Room& structureRoom = structureRoomPool.getRoom(structureEnteredID);

    Cursor::updateTileCursorInRoom(window, camera, dt, room, InventoryGUI::getHeldItemType(inventory), player.getTool());

    if (!isStateTransitioning())
    {
        player.updateInRoom(dt, Cursor::getMouseWorldPos(window, camera), room);
    }

    // Update room objects
    room.updateObjects(*this, dt);

    if (inStructure)
    {
        // Continue to update objects and entities in world
        chunkManager.updateChunksObjects(*this, dt);
        chunkManager.updateChunksEntities(dt, projectileManager, inventory, *this);

        if (!isStateTransitioning())
        {
            testExitStructure();
        }
    }
}

void Game::drawInRoom(float dt, const Room& room)
{
    // const Room& structureRoom = structureRoomPool.getRoom(structureEnteredID);
    room.draw(window, camera);

    std::vector<const WorldObject*> worldObjects = room.getObjects();
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

    switch (toolData.toolBehaviourType)
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
        selectedObject->damage(toolData.damage, *this, inventory, particleSystem);
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
    if (gameState != GameState::OnPlanet)
        return;

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
            inventory.addItem(fishCatchData.itemCatch, fishCatchData.count, true);
            break;
        }
    }
}

void Game::attemptObjectInteract()
{
    if (worldMenuState != WorldMenuState::Main && worldMenuState != WorldMenuState::Inventory)
    {
        return;
    }

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

        ObjectReference placeObjectReference = {Cursor::getSelectedChunk(chunkManager.getWorldSize()), Cursor::getSelectedChunkTile()};

        // Build object
        chunkManager.setObject(placeObjectReference.chunk, placeObjectReference.tile, objectType, *this);

        // Create build particles
        BuildableObject* placedObject = chunkManager.getChunkObject(placeObjectReference.chunk, placeObjectReference.tile);
        if (placedObject)
        {
            placedObject->createHitParticles(particleSystem);
        }
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

    const PlanetGenData& planetGenData = PlanetGenDataLoader::getPlanetGenData(chunkManager.getPlanetType());
    const BiomeGenData* biomeGenData = Chunk::getBiomeGenAtWorldTile(player.getWorldTileInside(chunkManager.getWorldSize()), chunkManager.getWorldSize(),
        chunkManager.getBiomeNoise(), chunkManager.getPlanetType());
    
    std::unordered_set<std::string> bossesSpawnAllowedNames = planetGenData.bossesSpawnAllowedNames;
    if (biomeGenData)
    {
        bossesSpawnAllowedNames.insert(biomeGenData->bossesSpawnAllowedNames.begin(), biomeGenData->bossesSpawnAllowedNames.end());
    }

    if (!bossesSpawnAllowedNames.contains(itemData.bossSummonData->bossName))
    {
        return;
    }

    // Summon boss
    if (!bossManager.createBoss(itemData.bossSummonData->bossName, player.getPosition(), *this))
    {
        return;
    }

    // Take boss summon item
    InventoryGUI::subtractHeldItem(inventory);
}

void Game::attemptUseConsumable()
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

    if (!itemData.consumableData.has_value())
    {
        return;
    }

    if (player.useConsumable(itemData.consumableData.value()))
    {
        InventoryGUI::subtractHeldItem(inventory);
    }
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
    if (chunkManager.canPlaceLand(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile()) && player.canReachPosition(Cursor::getMouseWorldPos(window, camera)))
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

    switch (gameState)
    {
        case GameState::OnPlanet:
        {
            return getObjectFromChunkOrRoom(ObjectReference{Cursor::getSelectedChunk(chunkManager.getWorldSize()), Cursor::getSelectedChunkTile()});
        }
        case GameState::InStructure: // fallthrough
        case GameState::InRoomDestination:
        {
            return getObjectFromChunkOrRoom(ObjectReference{{0, 0}, Cursor::getSelectedTile()});
        }
    }

    return nullptr;
}

BuildableObject* Game::getObjectFromChunkOrRoom(ObjectReference objectReference)
{
    switch (gameState)
    {
        case GameState::OnPlanet:
        {
            return chunkManager.getChunkObject(objectReference.chunk, objectReference.tile);
        }
        case GameState::InStructure:
        {
            Room& structureRoom = structureRoomPool.getRoom(structureEnteredID);
            return structureRoom.getObject(objectReference.tile);
        }
        case GameState::InRoomDestination:
        {
            return roomDestination.getObject(objectReference.tile);
        }   
    }

    return nullptr;
}


// -- Inventory / Chests -- //

void Game::giveStartingInventory()
{
    inventory.addItem(ItemDataLoader::getItemTypeFromName("Wooden Pickaxe"), 1);

    changePlayerTool();
}

void Game::handleInventoryClose()
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

void Game::openChest(ChestObject& chest)
{
    // If required
    InventoryGUI::shopClosed();

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

void Game::travelToDestination()
{
    travelTrigger = false;

    player.exitRocket();

    chestDataPool = ChestDataPool();
    structureRoomPool = RoomPool();
    bossManager.clearBosses();
    projectileManager.clear();
    enemyProjectileManager.clear();
    particleSystem.clear();
    landmarkManager.clear();
    nearbyCraftingStationLevels.clear();

    if (destinationPlanet >= 0)
    {
        travelToPlanet(destinationPlanet);
    }
    else if (destinationRoom >= 0)
    {
        travelToRoomDestination(destinationRoom);
    }

    destinationPlanet = 0;
    destinationRoom = 0;
}

void Game::travelToPlanet(PlanetType planetType)
{
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
    BuildableObject* rocketObject = getObjectFromChunkOrRoom(rocketEnteredReference);
    if (rocketObject)
    {
        rocketObject->triggerBehaviour(*this, ObjectBehaviourTrigger::RocketFlyDown);
    }
    camera.instantUpdate(player.getPosition());
}

void Game::travelToRoomDestination(RoomType destinationRoomType)
{
    overrideState(GameState::InRoomDestination);
    
    // roomDestinationManager.loadRoomDestinationType(destinationRoom, chestDataPool);
    GameSaveIO io(currentSaveFileSummary.name);

    RoomDestinationGameSave roomDestinationGameSave;

    if (io.loadRoomDestinationSave(destinationRoomType, roomDestinationGameSave))
    {
        chestDataPool = roomDestinationGameSave.chestDataPool;
        roomDestination = roomDestinationGameSave.roomDestination;
    }
    else
    {
        chestDataPool = ChestDataPool();
        roomDestination = Room(destinationRoomType, chestDataPool);
    }

    if (roomDestination.getFirstRocketObjectReference(rocketEnteredReference))
    {
        BuildableObject* rocketObject = getObjectFromChunkOrRoom(rocketEnteredReference);

        if (rocketObject)
        {
            player.setPosition(rocketObject->getPosition() - sf::Vector2f(TILE_SIZE_PIXELS_UNSCALED, 0));

            rocketObject->triggerBehaviour(*this, ObjectBehaviourTrigger::RocketFlyDown);
        }
    }
    else
    {
        std::cout << "Error: could not find rocket object in room destination\n";
    }

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

    chestDataPool = ChestDataPool();
    structureRoomPool = RoomPool();
    landmarkManager = LandmarkManager();
    particleSystem.clear();
    
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
                std::optional<sf::Vector2f> roomEntrancePos = structureRoomPool.getRoom(structureEnteredID).getEntrancePosition();

                //assert(roomEntrancePos.has_value());
                if (!roomEntrancePos.has_value())
                {
                    roomEntrancePos = sf::Vector2f(50, 50);
                }

                player.setPosition(roomEntrancePos.value());
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

    player = Player(sf::Vector2f(0, 0), &armourInventory);
    inventory = InventoryData(32);
    armourInventory = InventoryData(3);
    giveStartingInventory();

    chunkManager.setSeed(seed);

    initialiseNewPlanet(PlanetGenDataLoader::getPlanetTypeFromName("Earthlike"));

    dayCycleManager.setCurrentTime(dayCycleManager.getDayLength() * 0.5f);
    dayCycleManager.setCurrentDay(1);

    saveSessionPlayTime = 0.0f;

    bossManager.clearBosses();
    projectileManager.clear();
    enemyProjectileManager.clear();
    landmarkManager.clear();

    camera.instantUpdate(player.getPosition());

    chunkManager.updateChunks(*this, camera);
    lightingTick = LIGHTING_TICK;

    worldMenuState = WorldMenuState::Main;
    musicGap = MUSIC_GAP_MIN;
    startChangeStateTransition(GameState::OnPlanet);
}

bool Game::saveGame(bool gettingInRocket)
{
    if (gameState == GameState::MainMenu)
    {
        return false;
    }

    if (currentSaveFileSummary.name.empty())
    {
        return false;
    }

    GameSaveIO io(currentSaveFileSummary.name);

    PlayerGameSave playerGameSave;
    playerGameSave.seed = chunkManager.getSeed();
    playerGameSave.inventory = inventory;
    playerGameSave.armourInventory = armourInventory;
    playerGameSave.time = dayCycleManager.getCurrentTime();
    playerGameSave.day = dayCycleManager.getCurrentDay();

    // Add play time
    currentSaveFileSummary.timePlayed += std::round(saveSessionPlayTime);
    saveSessionPlayTime = 0.0f;
    playerGameSave.timePlayed = currentSaveFileSummary.timePlayed;

    PlanetGameSave planetGameSave;
    RoomDestinationGameSave roomDestinationGameSave;

    switch (gameState)
    {
        case GameState::InStructure:
        {
            planetGameSave.isInRoom = true;
            planetGameSave.inRoomID = structureEnteredID;
            planetGameSave.positionInRoom = player.getPosition();

            planetGameSave.playerLastPlanetPos = structureEnteredPos;
        } // fallthrough
        case GameState::OnPlanet:
        {
            planetGameSave.chunks = chunkManager.getChunkPODs();
            planetGameSave.chestDataPool = chestDataPool;
            planetGameSave.structureRoomPool = structureRoomPool;
            
            playerGameSave.planetType = chunkManager.getPlanetType();
            planetGameSave.playerLastPlanetPos = player.getPosition();
            
            if (gettingInRocket)
            {
                planetGameSave.rocketObjectUsed = rocketEnteredReference;
            }
            break;
        }
        case GameState::InRoomDestination:
        {
            roomDestinationGameSave.roomDestination = roomDestination;
            roomDestinationGameSave.chestDataPool = chestDataPool;
            roomDestinationGameSave.playerLastPos = player.getPosition();

            playerGameSave.roomDestinationType = roomDestination.getRoomType();
            break;
        }
    }

    io.writePlayerSave(playerGameSave);

    switch (gameState)
    {
        case GameState::InStructure: // fallthrough
        case GameState::OnPlanet:
        {
            io.writePlanetSave(playerGameSave.planetType, planetGameSave);
            break;
        }
        case GameState::InRoomDestination:
        {
            io.writeRoomDestinationSave(roomDestinationGameSave);
        }
    }

    return true;
}

bool Game::loadGame(const SaveFileSummary& saveFileSummary)
{
    GameSaveIO io(saveFileSummary.name);

    PlayerGameSave playerGameSave;
    
    if (!io.loadPlayerSave(playerGameSave))
    {
        std::cout << "Failed to load player " + saveFileSummary.name + "\n";
        return false;
    }

    player = Player(sf::Vector2f(0, 0), &armourInventory);

    closeChest();
    
    chunkManager.setSeed(playerGameSave.seed);
    inventory = playerGameSave.inventory;
    armourInventory = playerGameSave.armourInventory;
    dayCycleManager.setCurrentTime(playerGameSave.time);
    dayCycleManager.setCurrentDay(playerGameSave.day);

    changePlayerTool();

    GameState nextGameState = GameState::OnPlanet;
    worldMenuState = WorldMenuState::Main;
    musicGap = MUSIC_GAP_MIN;

    landmarkManager.clear();

    // Load planet
    if (playerGameSave.planetType >= 0)
    {
        PlanetGameSave planetGameSave;

        if (!io.loadPlanetSave(playerGameSave.planetType, planetGameSave))
        {
            const std::string& planetName = PlanetGenDataLoader::getPlanetGenData(playerGameSave.planetType).name;
            std::cout << "Failed to load planet \"" + planetName + "\" for save " + saveFileSummary.name + "\n";
            return false;
        }

        chunkManager.setPlanetType(playerGameSave.planetType);
        chunkManager.loadFromChunkPODs(planetGameSave.chunks, *this);
        chestDataPool = planetGameSave.chestDataPool;
        structureRoomPool = planetGameSave.structureRoomPool;

        nextGameState = GameState::OnPlanet;

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

        camera.instantUpdate(player.getPosition());

        chunkManager.updateChunks(*this, camera);
        lightingTick = LIGHTING_TICK;
    }
    else if (playerGameSave.roomDestinationType >= 0)
    {
        // Load room destination
        RoomDestinationGameSave roomDestinationGameSave;

        if (!io.loadRoomDestinationSave(playerGameSave.roomDestinationType, roomDestinationGameSave))
        {
            const std::string& roomDestinationName = StructureDataLoader::getRoomData(playerGameSave.roomDestinationType).name;
            std::cout << "Failed to load room \"" + roomDestinationName + "\" for save " + saveFileSummary.name + "\n";
            return false;
        }

        roomDestination = roomDestinationGameSave.roomDestination;
        chestDataPool = roomDestinationGameSave.chestDataPool;

        player.setPosition(roomDestinationGameSave.playerLastPos);

        nextGameState = GameState::InRoomDestination;
    }

    bossManager.clearBosses();
    projectileManager.clear();
    enemyProjectileManager.clear();
    particleSystem.clear();

    camera.instantUpdate(player.getPosition());

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

    if (!io.loadPlanetSave(planetType, planetGameSave))
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

void Game::saveOptions()
{
    OptionsSave optionsSave;
    optionsSave.musicVolume = Sounds::getMusicVolume();

    GameSaveIO optionsIO;
    optionsIO.writeOptionsSave(optionsSave);
}

void Game::loadOptions()
{
    OptionsSave optionsSave;

    GameSaveIO optionsIO;
    optionsIO.loadOptionsSave(optionsSave);

    Sounds::setMusicVolume(optionsSave.musicVolume);
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

    InputManager::processEvent(event);

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

    // Resize chest item slots
    if (worldMenuState == WorldMenuState::Inventory && openedChestID != 0xFFFF)
    {
        InventoryGUI::chestOpened(chestDataPool.getChestDataPtr(openedChestID));
    }

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

    ImGui::Spacing();

    static const int inputMaxLength = 100;
    static bool resetInputString = false;
    static char* itemToGive = new char[inputMaxLength];
    static int itemGiveAmount = 1;
    if (!resetInputString)
    {
        strcpy(itemToGive, "");
        itemToGive[inputMaxLength - 1] = '\0';
        resetInputString = true;
    }

    ImGui::InputText("Give item", itemToGive, 100);
    ImGui::InputInt("Give item amount", &itemGiveAmount);
    if (ImGui::Button("Give Item"))
    {
        inventory.addItem(ItemDataLoader::getItemTypeFromName(itemToGive), itemGiveAmount, true);
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