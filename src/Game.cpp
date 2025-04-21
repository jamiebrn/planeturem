#include "Game.hpp"

// FIX: Weather inconsistency (gametime)

// TODO: Make entities persistent across network and send state updates (movement, health etc)
// TODO: Use per-world entity chunk ID counter, rather than per chunk, to prevent collisions

// TODO: Improve entity movement over network

// TODO: Save all required planets
// TODO: Projectile networking
// TODO: Boss networking

// TODO: Maybe prevent building from hotbar when inventory open?

// TODO: Night and menu music

// PRIORITY: LOW
// TODO: Fishing rod draw order (split into separate objects?)
// FIX: Rocket particles on world wrap

// -- Public methods / entry point -- //

bool Game::initialise()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
    {
        std::cerr << "Failed to initialise SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);

    // Create window
    window.create(GAME_TITLE, displayMode.w * 0.75f, displayMode.h * 0.75f, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
    
    // Hide mouse cursor
    SDL_ShowCursor(SDL_DISABLE);

    // Set resolution handler values
    ResolutionHandler::setResolution({static_cast<uint32_t>(window.getWidth()), static_cast<uint32_t>(window.getHeight())});

    // Load assets
    if(!TextureManager::loadTextures()) return false;
    if(!Shaders::loadShaders()) return false;
    if(!TextDraw::loadFont("Data/Fonts/upheavtt.ttf", "Data/Shaders/default.vert", "Data/Shaders/font.frag")) return false;
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
    window.setIcon(icon);

    // Init ImGui
    #if (!RELEASE_BUILD)
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window.getSDLWindow(), window.getGLContext());
    ImGui_ImplOpenGL3_Init("#version 330 core");
    #endif

    // Load Steam API
    steamInitialised = SteamAPI_Init();
    // TODO: ugly
    Achievements::steamInitialised = steamInitialised;
    if (steamInitialised)
    {
        SteamUserStats()->RequestCurrentStats();
        SteamNetworkingUtils()->InitRelayNetworkAccess();
    }

    // Initialise network handler
    networkHandler.reset(this);

    // Randomise
    srand(time(NULL));

    loadOptions();

    InputManager::initialise(window.getSDLWindow());

    // Create key bindings
    InputManager::bindKey(InputAction::WALK_UP, SDL_Scancode::SDL_SCANCODE_W);
    InputManager::bindKey(InputAction::WALK_DOWN, SDL_Scancode::SDL_SCANCODE_S);
    InputManager::bindKey(InputAction::WALK_LEFT, SDL_Scancode::SDL_SCANCODE_A);
    InputManager::bindKey(InputAction::WALK_RIGHT, SDL_Scancode::SDL_SCANCODE_D);
    InputManager::bindKey(InputAction::OPEN_INVENTORY, SDL_Scancode::SDL_SCANCODE_E);
    InputManager::bindKey(InputAction::UI_BACK, SDL_Scancode::SDL_SCANCODE_ESCAPE);
    InputManager::bindKey(InputAction::UI_SHIFT, SDL_Scancode::SDL_SCANCODE_LSHIFT);
    InputManager::bindKey(InputAction::PAUSE_GAME, SDL_Scancode::SDL_SCANCODE_ESCAPE);
    InputManager::bindKey(InputAction::HOTBAR_0, SDL_Scancode::SDL_SCANCODE_1);
    InputManager::bindKey(InputAction::HOTBAR_1, SDL_Scancode::SDL_SCANCODE_2);
    InputManager::bindKey(InputAction::HOTBAR_2, SDL_Scancode::SDL_SCANCODE_3);
    InputManager::bindKey(InputAction::HOTBAR_3, SDL_Scancode::SDL_SCANCODE_4);
    InputManager::bindKey(InputAction::HOTBAR_4, SDL_Scancode::SDL_SCANCODE_5);
    InputManager::bindKey(InputAction::HOTBAR_5, SDL_Scancode::SDL_SCANCODE_6);
    InputManager::bindKey(InputAction::HOTBAR_6, SDL_Scancode::SDL_SCANCODE_7);
    InputManager::bindKey(InputAction::HOTBAR_7, SDL_Scancode::SDL_SCANCODE_8);
    InputManager::bindMouseButton(InputAction::USE_TOOL, SDL_BUTTON_LEFT);
    InputManager::bindMouseButton(InputAction::INTERACT, SDL_BUTTON_RIGHT);
    InputManager::bindMouseWheel(InputAction::ZOOM_IN, MouseWheelScroll::Up);
    InputManager::bindMouseWheel(InputAction::ZOOM_OUT, MouseWheelScroll::Down);
    InputManager::bindMouseWheel(InputAction::UI_TAB_LEFT, MouseWheelScroll::Down);
    InputManager::bindMouseWheel(InputAction::UI_TAB_RIGHT, MouseWheelScroll::Up);

    InputManager::bindControllerAxis(InputAction::WALK_UP, JoystickAxisWithDirection{SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY, JoystickAxisDirection::NEGATIVE});
    InputManager::bindControllerAxis(InputAction::WALK_DOWN, JoystickAxisWithDirection{SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY, JoystickAxisDirection::POSITIVE});
    InputManager::bindControllerAxis(InputAction::WALK_LEFT, JoystickAxisWithDirection{SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX, JoystickAxisDirection::NEGATIVE});
    InputManager::bindControllerAxis(InputAction::WALK_RIGHT, JoystickAxisWithDirection{SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX, JoystickAxisDirection::POSITIVE});
    InputManager::bindControllerAxis(InputAction::DIRECT_UP, JoystickAxisWithDirection{SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY, JoystickAxisDirection::NEGATIVE});
    InputManager::bindControllerAxis(InputAction::DIRECT_DOWN, JoystickAxisWithDirection{SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY, JoystickAxisDirection::POSITIVE});
    InputManager::bindControllerAxis(InputAction::DIRECT_LEFT, JoystickAxisWithDirection{SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX, JoystickAxisDirection::NEGATIVE});
    InputManager::bindControllerAxis(InputAction::DIRECT_RIGHT, JoystickAxisWithDirection{SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX, JoystickAxisDirection::POSITIVE});

    InputManager::bindControllerButton(InputAction::OPEN_INVENTORY, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B);
    InputManager::bindControllerButton(InputAction::UI_CONFIRM, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A);
    InputManager::bindControllerButton(InputAction::UI_CONFIRM_OTHER, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X);
    InputManager::bindControllerButton(InputAction::UI_BACK, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B);
    InputManager::bindControllerButton(InputAction::UI_SHIFT, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSTICK);
    InputManager::bindControllerButton(InputAction::RECENTRE_CONTROLLER_CURSOR, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSTICK);
    InputManager::bindControllerButton(InputAction::PAUSE_GAME, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START);
    InputManager::bindControllerAxis(InputAction::USE_TOOL, JoystickAxisWithDirection{SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT, JoystickAxisDirection::POSITIVE});
    InputManager::bindControllerAxis(InputAction::INTERACT, JoystickAxisWithDirection{SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT, JoystickAxisDirection::POSITIVE});
    InputManager::bindControllerButton(InputAction::UI_UP, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP);
    InputManager::bindControllerButton(InputAction::UI_DOWN, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    InputManager::bindControllerButton(InputAction::UI_LEFT, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    InputManager::bindControllerButton(InputAction::UI_RIGHT, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    InputManager::bindControllerButton(InputAction::ZOOM_IN, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    InputManager::bindControllerButton(InputAction::ZOOM_OUT, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    InputManager::bindControllerButton(InputAction::UI_TAB_LEFT, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    InputManager::bindControllerButton(InputAction::UI_TAB_RIGHT, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);

    InputManager::setControllerAxisDeadzone(0.3f);

    // Initialise values
    gameTime = 0;
    //mainMenuState = MainMenuState::Main;
    gameState = GameState::MainMenu;
    destinationGameState = gameState;
    transitionGameStateTimer = 0.0f;
    worldMenuState = WorldMenuState::Main;

    initialiseWorldData(0);
    locationState.setToNull();
    mainMenuGUI.initialise();

    //menuScreenshotIndex = 0;
    //menuScreenshotTimer = 0.0f;

    openedChestID = 0xFFFF;

    musicGapTimer = 0.0f;
    musicGap = 0.0f;

    player = Player(pl::Vector2f(0, 0));
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
    // pl::Vector2f spawnPos = chunkManager.findValidSpawnPosition(2);
    // player.setPosition(spawnPos);

    // Initialise inventory
    // giveStartingInventory();

    camera.instantUpdate(player.getPosition());

    // Return true by default
    return true;
}

void Game::deinit()
{
    #if (!RELEASE_BUILD)
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    #endif

    worldDatas.clear();
    lightingEngine.~LightingEngine();

    TextureManager::unloadTextures();
    TextDraw::unloadFont();
    Sounds::unloadSounds();
    pl::Sound::deinit();

    window.~Window();

    SDL_Quit();
}

void Game::run()
{
    std::chrono::high_resolution_clock clock;
    auto nowTime = clock.now();
    auto lastTime = nowTime;

    while (window.isOpen())
    {
        nowTime = clock.now();
        float dt = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime).count() / 1000000.0f;
        lastTime = nowTime;
        gameTime += dt;

        SteamAPI_RunCallbacks();

        Sounds::update(dt);
        
        InputManager::update();
        mouseScreenPos = InputManager::getMousePosition(window.getSDLWindow(), dt);

        if (networkHandler.isMultiplayerGame())
        {
            networkHandler.receiveMessages();
        }

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

        pl::TextDrawData textDrawData;
        textDrawData.size = 24;
        textDrawData.containOnScreenX = true;
        textDrawData.containPaddingRight = 10;
        textDrawData.position = pl::Vector2f(window.getWidth(), window.getHeight() - 40);
        textDrawData.text = Helper::floatToString(networkHandler.getTotalBytesReceived() / 1000.0f, 1) + "kb received (" +
            Helper::floatToString(networkHandler.getByteReceiveRate(dt) / 1000.0f, 1) + "KB/s)";
        
        TextDraw::drawText(window, textDrawData);
        
        textDrawData.position = pl::Vector2f(window.getWidth(), window.getHeight() - 65);
        textDrawData.text = Helper::floatToString(networkHandler.getTotalBytesSent() / 1000.0f, 1) + "kb sent (" +
            Helper::floatToString(networkHandler.getByteSendRate(dt) / 1000.0f, 1) + "KB/s)";

        TextDraw::drawText(window, textDrawData);

        #if (!RELEASE_BUILD)
        drawDebugMenu(dt);
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        #endif
        
        drawMouseCursor();

        // window.display();
        window.swapBuffers();
        window.showWindow();
        window.setVSync(ResolutionHandler::getVSync());
    }

    SDL_Quit();
}

// -- Main Menu -- //

void Game::runMainMenu(float dt)
{
    SDL_Event event;
    while (window.pollEvent(event))
    {
        handleEventsWindow(event);

        mainMenuGUI.handleEvent(event);
    }

    // pl::Vector2f mouseScreenPos = static_cast<pl::Vector2f>(sf::Mouse::getPosition(window));

    mainMenuGUI.update(dt, mouseScreenPos, *this);

    window.clear(pl::Color(0, 0, 0, 255));

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
                // if (steamInitialised)
                // {
                //     createLobby();
                // }
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
                    // if (steamInitialised)
                    // {
                    //     createLobby();
                    // }
                }

                break;
            }
            case MainMenuEventType::JoinGame:
            {
                networkHandler.sendWorldJoinReply(menuEvent->saveFileSummary.playerName);
                break;
            }
            case MainMenuEventType::CancelJoinGame:
            {
                networkHandler.leaveLobby();
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
                break;
            }
        }
    }
}

// -- Main Game -- //

void Game::runInGame(float dt)
{
    // pl::Vector2f mouseScreenPos = static_cast<pl::Vector2f>(sf::Mouse::getPosition(window));

    bool shiftMode = InputManager::isActionActive(InputAction::UI_SHIFT);

    // Handle events
    SDL_Event event;
    while (window.pollEvent(event))
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
        // Left click / use tool
        {
            bool uiInteracted = false;

            if (InputManager::isActionJustActivated(InputAction::USE_TOOL))
            {
                switch (worldMenuState)
                {
                    case WorldMenuState::Main:
                    {
                        if (!InputManager::isControllerActive())
                        {
                            uiInteracted = InventoryGUI::handleLeftClickHotbar(mouseScreenPos);
                        }

                        if (uiInteracted)
                        {
                            changePlayerTool();
                        }
                        break;
                    }
                    case WorldMenuState::NPCShop: // fallthrough
                    case WorldMenuState::Inventory:
                    {
                        ItemType itemHeldBefore = InventoryGUI::getHeldItemType(inventory);
                        
                        if (InventoryGUI::isMouseOverUI(mouseScreenPos) && !InputManager::isControllerActive())
                        {
                            InventoryGUI::handleLeftClick(*this, mouseScreenPos, shiftMode, networkHandler,
                                inventory, armourInventory, getChestDataPool().getChestDataPtr(openedChestID));
                            uiInteracted = true;
                        }

                        if (itemHeldBefore != InventoryGUI::getHeldItemType(inventory))
                        {
                            changePlayerTool();
                        }
                        break;
                    }
                }

                if (uiInteracted)
                {
                    // Prevent use of tool after interacting with UI for short period
                    player.startUseToolTimer();
                }
            }

            if (InputManager::isActionActive(InputAction::USE_TOOL))
            {
                switch (worldMenuState)
                {
                    case WorldMenuState::Main:
                    {
                        if (!uiInteracted || InputManager::isControllerActive())
                        {
                            if (player.isUseToolTimerFinished())
                            {
                                player.startUseToolTimer();
                                attemptUseTool();
                                attemptBuildObject();
                                attemptPlaceLand();
                                attemptUseBossSpawn();
                                attemptUseConsumable();
                            }
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
                        
                        if (!uiInteracted && player.isUseToolTimerFinished())
                        {
                            player.startUseToolTimer();
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
                    if (InventoryGUI::isMouseOverUI(mouseScreenPos) && !InputManager::isControllerActive())
                    {
                        InventoryGUI::handleRightClick(*this, mouseScreenPos, shiftMode, networkHandler,
                            inventory, armourInventory, getChestDataPool().getChestDataPtr(openedChestID));
                        changePlayerTool();
                    }
                    else
                    {
                        attemptObjectInteract();
                    }
                    break;
            }
        }

        std::vector<bool> hotbarInputActivated = {
            InputManager::isActionJustActivated(InputAction::HOTBAR_0),
            InputManager::isActionJustActivated(InputAction::HOTBAR_1),
            InputManager::isActionJustActivated(InputAction::HOTBAR_2),
            InputManager::isActionJustActivated(InputAction::HOTBAR_3),
            InputManager::isActionJustActivated(InputAction::HOTBAR_4),
            InputManager::isActionJustActivated(InputAction::HOTBAR_5),
            InputManager::isActionJustActivated(InputAction::HOTBAR_6),
            InputManager::isActionJustActivated(InputAction::HOTBAR_7)
        };

        for (int i = 0; i < hotbarInputActivated.size(); i++)
        {
            if (hotbarInputActivated[i])
            {
                InventoryGUI::setHotbarSelectedIndex(i);
                changePlayerTool();   
            }
        }

        if (float zoom = InputManager::getActionAxisImmediateActivation(InputAction::ZOOM_IN, InputAction::ZOOM_OUT);
            std::abs(zoom) >= 0.5f)
        {
            if ((worldMenuState == WorldMenuState::Inventory || worldMenuState == WorldMenuState::NPCShop) &&
                (!InventoryGUI::isMouseOverUI(mouseScreenPos) || InputManager::isControllerActive()))
            {
                if (InputManager::isControllerActive())
                {
                    handleZoom(-zoom);
                }
                else
                {
                    handleZoom(zoom);
                }
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
                {
                    InventoryGUI::shopClosed();
                }
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
                mainMenuGUI.initialisePauseMenu();
                InputManager::consumeInputAction(InputAction::PAUSE_GAME);
            }
        }
        
        if (InputManager::isActionJustActivated(InputAction::UI_BACK))
        {
            switch (worldMenuState)
            {
                case WorldMenuState::SettingLandmark: // fallthrough
                {
                    if (landmarkSetGUI.getGUIContext().isElementActive())
                    {
                        break;
                    }
                }
                case WorldMenuState::PauseMenu:
                {
                    if (worldMenuState == WorldMenuState::PauseMenu && mainMenuGUI.getGUIContext().isElementActive())
                    {
                        break;
                    }
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

        if (InputManager::isActionJustActivated(InputAction::RECENTRE_CONTROLLER_CURSOR))
        {
            InputManager::recentreControllerCursor(window.getSDLWindow());
        }
    }

    //
    // -- NETWORKING --
    //
    
    if (networkHandler.isMultiplayerGame())
    {
        networkHandler.update(dt);
        networkHandler.updateNetworkPlayers(dt, locationState);
        networkHandler.sendGameUpdates(dt, camera);
    }

    //
    // -- UPDATING --
    //

    if (worldMenuState != WorldMenuState::PauseMenu || (networkHandler.isMultiplayerGame() && networkHandler.getNetworkPlayerCount() > 0))
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

        player.setArmourFromInventory(armourInventory);

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
                Room& structureRoom = getStructureRoomPool().getRoom(locationState.getInStructureID());
                updateInRoom(dt, structureRoom, true);
                break;
            }
            case GameState::InRoomDestination:
            {
                updateInRoom(dt, getRoomDestination(), false);
                break;
            }
        }

        // Update active planets if host in multiplayer
        if (networkHandler.getIsLobbyHost())
        {
            updateActivePlanets(dt);
        }

        Cursor::setCursorHidden(!player.canReachPosition(camera.screenToWorldTransform(mouseScreenPos)));
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
                    InventoryGUI::updateInventory(mouseScreenPos, dt, inventory, armourInventory, getChestDataPool().getChestDataPtr(openedChestID));
                    break;
                }
            }
        }
    }

    //
    // -- DRAWING --
    //

    window.clear(pl::Color(0, 0, 0));

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
            const Room& structureRoom = getStructureRoomPool().getRoom(locationState.getInStructureID());
            drawInRoom(dt, structureRoom);
            break;
        }
        case GameState::InRoomDestination:
        {
            drawInRoom(dt, getRoomDestination());
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
            {
                InventoryGUI::drawHotbar(window, spriteBatch, mouseScreenPos, inventory);
                InventoryGUI::drawItemPopups(window, gameTime);
                HealthGUI::drawHealth(window, spriteBatch, player, gameTime, extraInfoStrings);

                // Controller glyphs
                if (InputManager::isControllerActive())
                {
                    std::vector<std::pair<InputAction, std::string>> actionStrings = {
                        {InputAction::RECENTRE_CONTROLLER_CURSOR, "Recentre Cursor"},
                        {InputAction::OPEN_INVENTORY, "Inventory"} 
                    };

                    drawControllerGlyphs(actionStrings);
                }

                break;
            }
            
            case WorldMenuState::NPCShop: // fallthrough
            case WorldMenuState::Inventory:
            {
                ItemType itemHeldBefore = InventoryGUI::getHeldItemType(inventory);
                if (InventoryGUI::handleControllerInput(*this, networkHandler, inventory, armourInventory, getChestDataPool().getChestDataPtr(openedChestID)))
                {
                    if (itemHeldBefore != InventoryGUI::getHeldItemType(inventory))
                    {
                        changePlayerTool();
                    }
                }
                HealthGUI::drawHealth(window, spriteBatch, player, gameTime, extraInfoStrings);
                spriteBatch.endDrawing(window);
                InventoryGUI::drawItemPopups(window, gameTime);

                InventoryData* chestDataPtr = nullptr;

                if (worldMenuState == WorldMenuState::Inventory)
                {
                    chestDataPtr = getChestDataPool().getChestDataPtr(openedChestID);
                }

                InventoryGUI::draw(window, spriteBatch, gameTime, mouseScreenPos, inventory, armourInventory, chestDataPtr);

                // Controller glyphs
                if (InputManager::isControllerActive())
                {
                    std::vector<std::pair<InputAction, std::string>> actionStrings = {
                        {InputAction::UI_SHIFT, "Quick Transfer"},
                        {InputAction::ZOOM_OUT, "Zoom Out"},  
                        {InputAction::ZOOM_IN, "Zoom In"},  
                        {InputAction::UI_CONFIRM_OTHER, "Select 1"},  
                        {InputAction::UI_CONFIRM, "Select All"},  
                    };

                    drawControllerGlyphs(actionStrings);
                }
                break;
            }
            
            
            case WorldMenuState::TravelSelect:
            {
                // std::vector<PlanetType> availableDestinations = getRocketAvailableDestinations();
                if (travelSelectGUI.createAndDraw(window, dt, destinationLocationState))
                {
                    BuildableObject* rocketObject = getObjectFromLocation(rocketEnteredReference, locationState);

                    if (rocketObject)
                    {
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
                    LandmarkObject* landmarkObjectPtr = getObjectFromLocation<LandmarkObject>(landmarkSetGUI.getLandmarkObjectReference(), locationState);
                    if (landmarkObjectPtr)
                    {
                        landmarkObjectPtr->setLandmarkColour(landmarkSetGUI.getColorA(), landmarkSetGUI.getColorB());
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
        }
    }
    else
    {
        HealthGUI::drawDeadPrompt(window);
    }

    if (worldMenuState == WorldMenuState::PauseMenu)
    {
        std::optional<PauseMenuEventType> pauseMenuEvent = mainMenuGUI.createAndDrawPauseMenu(window, dt, gameTime, steamInitialised, networkHandler.getLobbyID());

        if (pauseMenuEvent.has_value())
        {
            switch (pauseMenuEvent.value())
            {
                case PauseMenuEventType::Resume:
                {
                    worldMenuState = WorldMenuState::Main;
                    break;
                }
                case PauseMenuEventType::StartMultiplayer:
                {
                    networkHandler.startHostServer();
                    break;
                }
                case PauseMenuEventType::SaveOptions:
                {
                    saveOptions();
                    break;
                }
                case PauseMenuEventType::Quit:
                {
                    quitWorld();
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
    // pl::Vector2f mouseScreenPos = static_cast<pl::Vector2f>(sf::Mouse::getPosition(window));

    int worldSize = getChunkManager().getWorldSize();

    // Update cursor
    Cursor::updateTileCursor(camera.screenToWorldTransform(mouseScreenPos), dt, getChunkManager(), player.getCollisionRect(), InventoryGUI::getHeldItemType(inventory),
        player.getTool());

    // Update player
    bool wrappedAroundWorld = false;
    pl::Vector2f wrapPositionDelta;

    if (!isStateTransitioning())
    {
        player.update(dt, camera.screenToWorldTransform(mouseScreenPos), getChunkManager(), getProjectileManager(), wrappedAroundWorld, wrapPositionDelta);
    }

    // Handle world wrapping for camera and cursor, if player wrapped around
    if (wrappedAroundWorld)
    {
        camera.handleWorldWrap(wrapPositionDelta);
        Cursor::handleWorldWrap(wrapPositionDelta);
        // handleOpenChestPositionWorldWrap(wrapPositionDelta);
        getChunkManager().reloadChunks(camera.getChunkViewRange());

        // Wrap bosses
        getBossManager().handleWorldWrap(wrapPositionDelta);

        // Wrap projectiles
        getProjectileManager().handleWorldWrap(wrapPositionDelta);
        // enemyProjectileManager.handleWorldWrap(wrapPositionDelta);

        // Wrap hit markers
        HitMarkers::handleWorldWrap(wrapPositionDelta);

        // Wrap particles
        particleSystem.handleWorldWrap(wrapPositionDelta);

        // Wrap weather particles
        weatherSystem.handleWorldWrap(wrapPositionDelta);
    }

    // Update (loaded) chunks
    // Enable / disable chunk generation depending on multiplayer state
    if (!networkHandler.isMultiplayerGame() || networkHandler.isClient())
    {
        std::vector<ChunkPosition> chunksToRequestFromHost;
        
        bool hasLoadedChunks = getChunkManager().updateChunks(*this, player.getPosition(),
            {camera.getChunkViewRange()}, networkHandler.isClient(), &chunksToRequestFromHost);
        bool hasUnloadedChunks = getChunkManager().unloadChunksOutOfView({camera.getChunkViewRange()});
    
        if (networkHandler.isClient() && chunksToRequestFromHost.size() > 0)
        {
            networkHandler.requestChunksFromHost(locationState.getPlanetType(), chunksToRequestFromHost);
        }
    
        getChunkManager().updateChunksObjects(*this, dt);
        getChunkManager().updateChunksEntities(dt, getProjectileManager(), *this, networkHandler.isClient());
    
        // If modified chunks, force a lighting recalculation
        if (hasLoadedChunks || hasUnloadedChunks)
        {
            lightingTick = LIGHTING_TICK;
        }
    }
    
    // Get nearby crafting stations
    nearbyCraftingStationLevels = getChunkManager().getNearbyCraftingStationLevels(player.getChunkInside(worldSize), player.getChunkTileInside(worldSize), 4);

    // Update bosses
    getBossManager().update(*this, getProjectileManager(), getChunkManager(), player, dt, gameTime);

    // Update projectiles
    getProjectileManager().update(dt);
    // enemyProjectileManager.update(dt);

    weatherSystem.update(dt, gameTime, camera, getChunkManager());

    // Test item pickups colliding
    std::optional<ItemPickupReference> itemPickupColliding = getChunkManager().getCollidingItemPickup(player.getCollisionRect(), gameTime);
    if (itemPickupColliding.has_value())
    {
        const ItemPickup* itemPickupPtr = getChunkManager().getChunk(itemPickupColliding->chunk)->getItemPickup(itemPickupColliding->id);

        if (itemPickupPtr != nullptr)
        {
            // Only actually add item to inventory if solo or is host
            // Host will give client item
            bool modifyInventory = networkHandler.isLobbyHostOrSolo();

            int amountAdded = inventory.addItem(itemPickupPtr->getItemType(), 1, modifyInventory, false, modifyInventory);

            if (amountAdded > 0)
            {
                // Only delete pickup if playing solo or is host
                // if (!multiplayerGame || isLobbyHost)
                // {
                getChunkManager().deleteItemPickup(itemPickupColliding.value());
                // }
    
                // Play pickup sound
                const std::vector<SoundType> pickupSounds = {SoundType::Pop0, SoundType::Pop1, SoundType::Pop2, SoundType::Pop3};
                Sounds::playSound(pickupSounds[Helper::randInt(0, pickupSounds.size() - 1)], 30.0f);

                // Networking
                if (networkHandler.isMultiplayerGame() && networkHandler.getNetworkPlayerCount() > 0)
                {
                    PacketDataItemPickupDeleted packetData;
                    packetData.locationState = locationState;
                    packetData.pickupDeleted = itemPickupColliding.value();
                    Packet packet;
                    packet.set(packetData);

                    if (networkHandler.getIsLobbyHost())
                    {
                        networkHandler.sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
                    }
                    else
                    {
                        networkHandler.sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
                    }

                    // Queue send player data if required
                    if (modifyInventory)
                    {
                        networkHandler.queueSendPlayerData();
                    }
                }
            }
        }
    }

    if (!isStateTransitioning() && !player.isInRocket() && player.isAlive())
    {
        testEnterStructure();
    }
}

void Game::updateActivePlanets(float dt)
{
    // Host function
    // Update all active planets for all clients and view ranges

    std::unordered_set<PlanetType> planetTypeSet = networkHandler.getPlayersPlanetTypeSet(locationState.getPlanetType());

    for (PlanetType planetType : planetTypeSet)
    {
        std::vector<ChunkViewRange> chunkViewRanges = networkHandler.getNetworkPlayersChunkViewRanges(planetType);

        std::optional<pl::Vector2f> localPlayerPosition;

        // Add this player (host) chunk view range and set player position, if also on this planet
        if (locationState.getPlanetType() == planetType)
        {
            chunkViewRanges.push_back(camera.getChunkViewRange());
            localPlayerPosition = player.getPosition();
        }

        bool hasLoadedChunks = getChunkManager(planetType).updateChunks(*this, localPlayerPosition, chunkViewRanges);
        bool hasUnloadedChunks = getChunkManager(planetType).unloadChunksOutOfView(chunkViewRanges);
    
        getChunkManager(planetType).updateChunksObjects(*this, dt);
        getChunkManager(planetType).updateChunksEntities(dt, getProjectileManager(planetType), *this, false);

        // If modified chunks (and this player (host) is on this planet), force a lighting recalculation
        if (locationState.getPlanetType() == planetType && (hasLoadedChunks || hasUnloadedChunks))
        {
            lightingTick = LIGHTING_TICK;
        }
    }
}

void Game::drawOnPlanet(float dt)
{
    // Get world objects
    ChunkViewRange chunkViewRange = camera.getChunkViewRange();

    std::vector<WorldObject*> worldObjects = getChunkManager().getChunkObjects(chunkViewRange);
    std::vector<WorldObject*> entities = getChunkManager().getChunkEntities(chunkViewRange);
    std::vector<WorldObject*> itemPickups = getChunkManager().getItemPickups(chunkViewRange);
    std::vector<WorldObject*> weatherParticles = weatherSystem.getWeatherParticles();
    worldObjects.insert(worldObjects.end(), entities.begin(), entities.end());
    worldObjects.insert(worldObjects.end(), itemPickups.begin(), itemPickups.end());
    worldObjects.insert(worldObjects.end(), weatherParticles.begin(), weatherParticles.end());
    worldObjects.push_back(&player);
    getBossManager().getBossWorldObjects(worldObjects);

    // Add network players
    if (networkHandler.isMultiplayerGame())
    {
        std::vector<WorldObject*> networkPlayerObjects = networkHandler.getNetworkPlayersToDraw(locationState, player.getPosition());
        worldObjects.insert(worldObjects.end(), networkPlayerObjects.begin(), networkPlayerObjects.end());
    }
    
    drawWorld(worldTexture, dt, worldObjects, worldDatas.at(locationState.getPlanetType()), camera);
    drawLighting(dt, worldObjects);

    // UI
    // pl::Vector2f mouseScreenPos = static_cast<pl::Vector2f>(sf::Mouse::getPosition(window));

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

    getBossManager().drawStatsAtCursor(window, camera, mouseScreenPos);
}

void Game::drawWorld(pl::Framebuffer& renderTexture, float dt, std::vector<WorldObject*>& worldObjects, WorldData& worldData, const Camera& cameraArg)
{
    // Draw all world onto texture for lighting
    renderTexture.create(window.getWidth(), window.getHeight());
    renderTexture.clear(pl::Color(0, 0, 0));

    // Draw water
    worldData.chunkManager.drawChunkWater(renderTexture, cameraArg, gameTime);

    std::sort(worldObjects.begin(), worldObjects.end(), [](WorldObject* a, WorldObject* b)
    {
        if (a->getDrawLayer() != b->getDrawLayer()) return a->getDrawLayer() > b->getDrawLayer();
        if (a->getPosition().y == b->getPosition().y) return a->getPosition().x < b->getPosition().x;
        return a->getPosition().y < b->getPosition().y;
    });

    spriteBatch.beginDrawing();

    // Draw terrain
    worldData.chunkManager.drawChunkTerrain(renderTexture, spriteBatch, cameraArg, gameTime);

    // Draw particles
    particleSystem.draw(renderTexture, spriteBatch, cameraArg);

    // Draw objects
    for (WorldObject* worldObject : worldObjects)
    {
        worldObject->draw(renderTexture, spriteBatch, *this, cameraArg, dt, gameTime, worldData.chunkManager.getWorldSize(), {255, 255, 255, 255});
    }

    // Draw projectiles
    worldData.projectileManager.drawProjectiles(renderTexture, spriteBatch, worldData.chunkManager, player.getPosition(), cameraArg);
    // enemyProjectileManager.drawProjectiles(renderTexture, spriteBatch, cameraArg);

    spriteBatch.endDrawing(renderTexture);

    // renderTexture.display();
}

void Game::drawLighting(float dt, std::vector<WorldObject*>& worldObjects)
{
    float lightLevel = dayCycleManager.getLightLevel();

    float ambientRedLight = Helper::lerp(2, 255 * weatherSystem.getRedLightBias(), lightLevel);
    float ambientGreenLight = Helper::lerp(7, 244 * weatherSystem.getGreenLightBias(), lightLevel);
    float ambientBlueLight = Helper::lerp(14, 234 * weatherSystem.getBlueLightBias(), lightLevel);

    pl::Vector2<int> chunksSizeInView = getChunkManager().getChunksSizeInView(camera);
    pl::Vector2f topLeftChunkPos = getChunkManager().topLeftChunkPosInView(camera);
    
    // Draw light sources on light texture
    pl::Framebuffer lightTexture;
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

        lightingEngine.calculateLighting();
    }


    lightTexture.clear({ambientRedLight, ambientGreenLight, ambientBlueLight, 255});

    // draw from lighting engine
    lightingEngine.drawLighting(lightTexture, pl::Color(255, 220, 140));

    lightTexture.setLinearFilter(smoothLighting);

    pl::VertexArray lightRect;
    lightRect.addQuad(pl::Rect<float>(camera.worldToScreenTransform(topLeftChunkPos), pl::Vector2f(lightTexture.getWidth(), lightTexture.getHeight()) *
        ResolutionHandler::getScale() * TILE_SIZE_PIXELS_UNSCALED / static_cast<float>(TILE_LIGHTING_RESOLUTION)), pl::Color(),
        pl::Rect<float>(0, lightTexture.getHeight(), lightTexture.getWidth(), -lightTexture.getHeight()));

    worldTexture.draw(lightRect, *Shaders::getShader(ShaderType::Default), &lightTexture.getTexture(), pl::BlendMode::Multiply);

    pl::VertexArray worldRect;
    worldRect.addQuad(pl::Rect<float>(0, 0, worldTexture.getWidth(), worldTexture.getHeight()), pl::Color(),
        pl::Rect<float>(0, worldTexture.getHeight(), worldTexture.getWidth(), -worldTexture.getHeight()));

    window.draw(worldRect, *Shaders::getShader(ShaderType::Default), &worldTexture.getTexture(), pl::BlendMode::Alpha);
}

void Game::drawLandmarks()
{
    AnimatedTexture landmarkUIAnimation(6, 16, 16, 96, 112, 0.1);

    landmarkUIAnimation.setFrame(static_cast<int>(gameTime / 0.1) % 6);

    static const int PADDING = 0;
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    pl::Vector2<uint32_t> resolution = ResolutionHandler::getResolution();

    pl::DrawData drawData;
    drawData.texture = TextureManager::getTexture(TextureType::UI);
    drawData.shader = Shaders::getShader(ShaderType::ReplaceColour);
    drawData.scale = pl::Vector2f(3, 3) * intScale;
    drawData.centerRatio = pl::Vector2f(0.5f, 0.5f);
    drawData.color = pl::Color(255, 255, 255, 150);
    
    std::vector<float> replaceKeys = {1, 1, 1, 1, 0, 0, 0, 1};
    drawData.shader->setUniform1i("replaceKeyCount", replaceKeys.size() / 4);
    drawData.shader->setUniform4fv("replaceKeys", replaceKeys);
    
    for (const LandmarkSummaryData& landmarkSummary : getLandmarkManager().getLandmarkSummaryDatas(player, getChunkManager()))
    {
        if (camera.isInView(landmarkSummary.worldPos))
        {
            continue;
        }
        
        pl::Vector2f screenPos = camera.worldToScreenTransform(landmarkSummary.worldPos);
        screenPos.x = std::clamp(screenPos.x, PADDING * intScale, resolution.x - PADDING * intScale);
        screenPos.y = std::clamp(screenPos.y, PADDING * intScale, resolution.y - PADDING * intScale);
        
        pl::Color colorANormalised = landmarkSummary.colorA.normalise();
        pl::Color colorBNormalised = landmarkSummary.colorB.normalise();
        
        std::vector<float> replaceValues = {
            colorANormalised.r, colorANormalised.g, colorANormalised.b, colorANormalised.a,
            colorBNormalised.r, colorBNormalised.g, colorBNormalised.b, colorBNormalised.a
        };
        
        drawData.position = screenPos;
        drawData.shader->setUniform4fv("replaceValues", replaceValues);
        drawData.textureRect = landmarkUIAnimation.getTextureRect();

        spriteBatch.draw(window, drawData);
        spriteBatch.endDrawing(window);
    }
}


// Structure
std::optional<uint32_t> Game::initialiseStructureOrGet(PlanetType planetType, ChunkPosition chunk, pl::Vector2f* entrancePos, RoomType* roomType)
{
    assert(networkHandler.isLobbyHostOrSolo());

    if (!worldDatas.contains(planetType))
    {
        printf(("ERROR: Attempted to access null planet type " + std::to_string(planetType) + " while initialising structure" + "\n").c_str());
        return std::nullopt;
    }

    Chunk* chunkPtr = getChunkManager(planetType).getChunk(chunk);
    if (!chunkPtr)
    {
        printf(("ERROR: Attempted to access null chunk " + chunk.toString() + " while initialising structure" + "\n").c_str());
        return std::nullopt;
    }

    StructureObject* enteredStructure = chunkPtr->getStructureObject();
    if (!enteredStructure)
    {
        printf(("ERROR: Attempted to initialise null structure in chunk " + chunk.toString() + "\n").c_str());
        return std::nullopt;
    }

    uint32_t structureID = enteredStructure->getStructureID();
    const StructureData& structureData = StructureDataLoader::getStructureData(enteredStructure->getStructureType());
    
    // Initialise
    if (structureID == 0xFFFFFFFF)
    {
        structureID = getStructureRoomPool(planetType).createRoom(structureData.roomType, getChestDataPool(LocationState::createFromPlanetType(planetType)));
        enteredStructure->setStructureID(structureID);
    }

    if (entrancePos)
    {
        // TODO: May break chunk positions when transmitting to client
        pl::Vector2f structureEntrancePos = enteredStructure->getEntrancePosition();
        entrancePos->x = (std::floor(structureEntrancePos.x / TILE_SIZE_PIXELS_UNSCALED) + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
        entrancePos->y = (std::floor(structureEntrancePos.y / TILE_SIZE_PIXELS_UNSCALED) + 1.5f) * TILE_SIZE_PIXELS_UNSCALED;
    }
    if (roomType)
    {
        *roomType = structureData.roomType;
    }
    
    return structureID;
}

void Game::testEnterStructure()
{
    std::optional<ChunkPosition> structureEnteredChunk = getChunkManager().isPlayerInStructureEntrance(player.getPosition());

    if (!structureEnteredChunk.has_value())
    {
        return;
    }
    
    // Structure has been entered

    // If client, request structure enter from host
    if (networkHandler.isClient())
    {
        if (networkHandler.canSendStructureRequest())
        {
            PacketDataStructureEnterRequest packetDataRequest;
            packetDataRequest.planetType = locationState.getPlanetType();
            packetDataRequest.chunkPos = structureEnteredChunk.value();
            Packet packet;
            packet.set(packetDataRequest);
            networkHandler.sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
            networkHandler.structureRequestSent();
        }
        return;
    }

    // Host / solo
    std::optional<uint32_t> structureID = initialiseStructureOrGet(locationState.getPlanetType(), structureEnteredChunk.value(), &structureEnteredPos, nullptr);
    if (!structureID.has_value())
    {
        return;
    }

    locationState.setInStructureID(structureID.value());

    // changeState(GameState::InStructure);
    startChangeStateTransition(GameState::InStructure);
}

void Game::enterStructureFromHost(PlanetType planetType, ChunkPosition chunk, uint32_t structureID, pl::Vector2f entrancePos, RoomType roomType)
{
    if (locationState.getPlanetType() != planetType)
    {
        printf(("ERROR: Received enter structure reply for incorrect planet type " + std::to_string(planetType) + "\n").c_str());
        return;
    }

    Chunk* chunkPtr = getChunkManager(planetType).getChunk(chunk);
    if (!chunkPtr)
    {
        printf(("ERROR: Received enter structure reply for null chunk " + chunk.toString() + "\n").c_str());
        return;
    }

    StructureObject* structureObject = chunkPtr->getStructureObject();
    if (!structureObject)
    {
        printf("ERROR: Received enter structure reply for null structure\n");
        return;
    }

    printf("NETWORK: Entering structure from host\n");

    structureObject->setStructureID(structureID);
    getStructureRoomPool(planetType).overwriteRoomData(structureID, Room(roomType, nullptr));
    structureEnteredPos = entrancePos;

    locationState.setInStructureID(structureID);

    startChangeStateTransition(GameState::InStructure);
}

void Game::testExitStructure()
{
    const Room& structureRoom = getStructureRoomPool().getRoom(locationState.getInStructureID());
    
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
            rocketEnteredReference.chunk = rocket.getChunkInside(getChunkManager().getWorldSize());
            rocketEnteredReference.tile = rocket.getChunkTileInside(getChunkManager().getWorldSize());

            // Add to used rockets
            planetRocketUsedPositions[locationState.getPlanetType()] = rocketEnteredReference;
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

    // PlanetType currentPlanetType = -1;
    // RoomType currentRoomType = -1;
    // if (gameState == GameState::OnPlanet)
    // {
    //     currentPlanetType = chunkManager.getPlanetType();
    // }
    // else if (gameState == GameState::InRoomDestination)
    // {
    //     currentRoomType = roomDestination.getRoomType();
    // }

    std::vector<PlanetType> planetDestinations;
    std::vector<RoomType> roomDestinations;

    rocket.getRocketAvailableDestinations(locationState.getPlanetType(), locationState.getRoomDestType(), planetDestinations, roomDestinations);

    travelSelectGUI.setAvailableDestinations(planetDestinations, roomDestinations);

    player.enterRocket(rocket.getRocketPosition());
}

void Game::exitRocket()
{
    worldMenuState = WorldMenuState::Main;

    // Get rocket object
    BuildableObject* rocketObject = getObjectFromLocation(rocketEnteredReference, locationState);

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
    ObjectReference landmarkObjectReference = ObjectReference{landmark.getChunkInside(getChunkManager().getWorldSize()),
        landmark.getChunkTileInside(getChunkManager().getWorldSize())};

    getLandmarkManager().addLandmark(landmarkObjectReference);

    if (createGUI)
    {
        handleInventoryClose();
        worldMenuState = WorldMenuState::SettingLandmark;
        landmarkSetGUI.initialise(landmarkObjectReference, landmark.getColorA(), landmark.getColorB());

        player.setCanMove(false);
    }
}

void Game::landmarkDestroyed(const LandmarkObject& landmark)
{
    getLandmarkManager().removeLandmark(ObjectReference{landmark.getChunkInside(getChunkManager().getWorldSize()),
        landmark.getChunkTileInside(getChunkManager().getWorldSize())});
}

// -- In Room -- //

void Game::updateInRoom(float dt, Room& room, bool inStructure)
{
    // pl::Vector2f mouseScreenPos = static_cast<pl::Vector2f>(sf::Mouse::getPosition(window));

    // Room& structureRoom = structureRoomPool.getRoom(structureEnteredID);

    Cursor::updateTileCursorInRoom(camera.screenToWorldTransform(mouseScreenPos), dt, room, InventoryGUI::getHeldItemType(inventory), player.getTool());

    if (!isStateTransitioning())
    {
        player.updateInRoom(dt, camera.screenToWorldTransform(mouseScreenPos), room);
    }

    // Update room objects
    room.updateObjects(*this, dt);

    if (inStructure)
    {
        // Continue to update objects and entities in world
        getChunkManager().updateChunksObjects(*this, dt);
        getChunkManager().updateChunksEntities(dt, getProjectileManager(), *this, networkHandler.isClient());
            
        weatherSystem.update(dt, gameTime, camera, getChunkManager());

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

    // Add network players
    if (networkHandler.isMultiplayerGame())
    {
        std::vector<WorldObject*> networkPlayerObjects = networkHandler.getNetworkPlayersToDraw(locationState, player.getPosition());
        worldObjects.insert(worldObjects.end(), networkPlayerObjects.begin(), networkPlayerObjects.end());
    }

    std::sort(worldObjects.begin(), worldObjects.end(), [](const WorldObject* a, const WorldObject* b)
    {
        if (a->getDrawLayer() != b->getDrawLayer()) return a->getDrawLayer() > b->getDrawLayer();
        if (a->getPosition().y == b->getPosition().y) return a->getPosition().x < b->getPosition().x;
        return a->getPosition().y < b->getPosition().y;
    });

    for (const WorldObject* object : worldObjects)
    {
        // Use 1 for worldsize as dummy value
        object->draw(window, spriteBatch, *this, camera, dt, gameTime, 1, pl::Color(255, 255, 255));
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

    if (player.isUsingTool() && !DebugOptions::crazyAttack)
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
    pl::Vector2f mouseWorldPos = camera.screenToWorldTransform(mouseScreenPos);
    
    // Swing pickaxe
    player.useTool(getProjectileManager(), inventory, mouseWorldPos, *this);

    if (gameState != GameState::OnPlanet)
        return;

    if (!player.canReachPosition(mouseWorldPos))
        return;

    // Get current tool damage amount
    ToolType currentTool = player.getTool();

    const ToolData& toolData = ToolDataLoader::getToolData(currentTool);

    hitObject(Cursor::getSelectedChunk(getChunkManager().getWorldSize()), Cursor::getSelectedChunkTile(), toolData.damage);
}

void Game::attemptUseToolFishingRod()
{
    if (gameState != GameState::OnPlanet)
        return;

    // Check if fish is biting line first - if so, reel in fishing rod and catch fish
    if (player.isFishBitingLine())
    {
        pl::Vector2<int> fishedTile = player.reelInFishingRod();
        catchRandomFish(fishedTile);
        return;
    }

    pl::Vector2f mouseWorldPos = camera.screenToWorldTransform(mouseScreenPos);

    if (!player.canReachPosition(mouseWorldPos))
    {
        player.reelInFishingRod();
        return;
    }
    
    // Determine whether can fish at selected tile
    ChunkPosition selectedChunk = Cursor::getSelectedChunk(getChunkManager().getWorldSize());
    pl::Vector2<int> selectedTile = Cursor::getSelectedChunkTile();

    // Test whether can fish on selected tile
    // Must have no object + be water
    BuildableObject* selectedObject = getChunkManager().getChunkObject(selectedChunk, selectedTile);
    int tileType = getChunkManager().getChunkTileType(selectedChunk, selectedTile);

    if (selectedObject || tileType > 0)
    {
        player.reelInFishingRod();
        return;
    }
    
    // Swing fishing rod
    player.useTool(getProjectileManager(), inventory, mouseWorldPos, *this);

    player.swingFishingRod(mouseWorldPos, Cursor::getSelectedWorldTile(getChunkManager().getWorldSize()));
}

void Game::attemptUseToolWeapon()
{
    if (gameState != GameState::OnPlanet)
        return;

    pl::Vector2f mouseWorldPos = camera.screenToWorldTransform(mouseScreenPos);

    player.useTool(getProjectileManager(), inventory, mouseWorldPos, *this);
}

void Game::hitObject(ChunkPosition chunk, pl::Vector2<int> tile, int damage, std::optional<PlanetType> planetType, bool sentFromHost, std::optional<uint64_t> userId)
{
    // If multiplayer and this client attempted to hit object, send hit object packet to host
    if (networkHandler.isClient() && !sentFromHost)
    {
        if (!locationState.isOnPlanet())
        {
            return;
        }

        PacketDataObjectHit packetData;
        packetData.planetType = locationState.getPlanetType();
        packetData.objectHit.chunk = chunk;
        packetData.objectHit.tile = tile;
        packetData.damage = damage;
        packetData.userId = SteamUser()->GetSteamID().ConvertToUint64();

        Packet packet;
        packet.set(packetData);
        networkHandler.sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
        return;
    }

    if (!planetType.has_value())
    {
        planetType = locationState.getPlanetType();
    }

    // Not multiplayer game / is host / hit object packet sent from host

    if (sentFromHost && planetType.value() != locationState.getPlanetType())
    {
        return;
    }

    bool canDestroyObject = getChunkManager(planetType).canDestroyObject(chunk, tile, player.getCollisionRect());

    if (!canDestroyObject)
        return;

    BuildableObject* selectedObject = getChunkManager(planetType).getChunkObject(chunk, tile);

    if (selectedObject)
    {
        // Only drop items if playing solo or is host of lobby
        // In multiplayer, host handles creation of all pickups and alerts clients of new pickups
        bool destroyed = selectedObject->damage(damage, *this, getChunkManager(planetType), particleSystem, networkHandler.isLobbyHostOrSolo());

        // Alert network players if host
        if (networkHandler.getIsLobbyHost())
        {
            PacketDataObjectHit packetData;
            packetData.planetType = planetType.value();
            packetData.objectHit.chunk = chunk;
            packetData.objectHit.tile = tile;
            packetData.damage = damage;
            if (userId.has_value())
            {
                packetData.userId = userId.value();
            }
            else
            {
                packetData.userId = SteamUser()->GetSteamID().ConvertToUint64();
            }

            Packet packet;
            packet.set(packetData);
            networkHandler.sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
        }

        if (destroyed)
        {
            // Only apply screenshake if this client destroyed object
            bool applyScreenShake = true;
            
            if (networkHandler.isMultiplayerGame())
            {
                // If host, alert clients of object destruction
                if (networkHandler.getIsLobbyHost())
                {
                    PacketDataObjectDestroyed objectDestroyedPacketData;
                    objectDestroyedPacketData.planetType = planetType.value();
                    objectDestroyedPacketData.objectReference.chunk = chunk;
                    objectDestroyedPacketData.objectReference.tile = tile;
                    objectDestroyedPacketData.userId = userId.has_value() ? userId.value() : SteamUser()->GetSteamID().ConvertToUint64(); 
                    Packet packet;
                    packet.set(objectDestroyedPacketData);
                    networkHandler.sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
                }
                
                if (userId.has_value())
                {
                    // Hit came from different client / host, do not screenshake
                    if (userId.value() != SteamUser()->GetSteamID().ConvertToUint64())
                    {
                        applyScreenShake = false;
                    }
                }
            }

            if (applyScreenShake)
            {
                camera.setScreenShakeTime(0.3f);
            }
        }
    }
}

void Game::buildObject(ChunkPosition chunk, pl::Vector2<int> tile, ObjectType objectType, std::optional<PlanetType> planetType, bool sentFromHost,
    bool builtByPlayer)
{
    // If multiplayer game and this client builds object, send build object packet to host
    if (networkHandler.isClient() && !sentFromHost)
    {
        if (!locationState.isOnPlanet())
        {
            return;
        }
        PacketDataObjectBuilt packetData;
        packetData.planetType = locationState.getPlanetType();
        packetData.objectReference.chunk = chunk;
        packetData.objectReference.tile = tile;
        packetData.objectType = objectType;
        packetData.builtByPlayer = builtByPlayer;
        packetData.userId = SteamUser()->GetSteamID().ConvertToUint64();

        Packet packet;
        packet.set(packetData);
        networkHandler.sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
        return;
    }

    if (!planetType.has_value())
    {
        planetType = locationState.getPlanetType();
    }

    if (sentFromHost && planetType.value() != locationState.getPlanetType())
    {
        return;
    }

    // Not multiplayer game / sent from host / is host

    if (networkHandler.getIsLobbyHost())
    {
        // Send build object packets to clients
        PacketDataObjectBuilt packetData;
        packetData.planetType = planetType.value();
        packetData.objectReference.chunk = chunk;
        packetData.objectReference.tile = tile;
        packetData.objectType = objectType;
        packetData.builtByPlayer = builtByPlayer;
        packetData.userId = SteamUser()->GetSteamID().ConvertToUint64();

        Packet packet;
        packet.set(packetData);
        networkHandler.sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
    }

    // If sent from host and client does not have chunk, request it
    if (networkHandler.isClient() && sentFromHost)
    {
        Chunk* chunkPtr = getChunkManager(planetType).getChunk(chunk);
        if (!chunkPtr)
        {
            std::vector<ChunkPosition> requestedChunks = {chunk};
            networkHandler.requestChunksFromHost(locationState.getPlanetType(), requestedChunks);
            return;
        }
    }

    // Build object
    getChunkManager(planetType).setObject(chunk, tile, objectType, *this);

    // If not built by player, don't create build particles or play build sound
    if (!builtByPlayer)
    {
        return;
    }

    // Create build particles
    BuildableObject* placedObject = getChunkManager(planetType).getChunkObject(chunk, tile);
    if (placedObject)
    {
        placedObject->createHitParticles(particleSystem);
    }
    
    // Play build sound if object in view
    if (camera.isInView(placedObject->getPosition()))
    {
        int soundChance = rand() % 2;
        SoundType buildSound = SoundType::CraftBuild1;
        if (soundChance == 1) buildSound = SoundType::CraftBuild2;
        Sounds::playSound(buildSound, 60.0f);
    }
}

void Game::destroyObjectFromHost(ChunkPosition chunk, pl::Vector2<int> tile, std::optional<PlanetType> planetType)
{
    if (locationState.getPlanetType() != planetType)
    {
        return;
    }

    getChunkManager(planetType).deleteObject(chunk, tile);
}

// TODO: Use client melee data / location state
void Game::testMeleeCollision(const std::vector<HitRect>& hitRects)
{
    getChunkManager().testChunkEntityHitCollision(hitRects, *this, gameTime);
    getBossManager().testHitRectCollision(hitRects);
}

void Game::itemPickupsCreated(const std::vector<ItemPickupReference>& itemPickupsCreated, std::optional<LocationState> pickupsLocationState)
{
    if (!networkHandler.getIsLobbyHost())
    {
        return;
    }

    if (!pickupsLocationState.has_value())
    {
        pickupsLocationState = locationState;
    }

    // Alert clients of item pickups created
    PacketDataItemPickupsCreated packetData;
    packetData.locationState = pickupsLocationState.value();
    
    for (auto& itemPickupReference : itemPickupsCreated)
    {
        Chunk* chunkPtr = getChunkManager(pickupsLocationState->getPlanetType()).getChunk(itemPickupReference.chunk);
        if (!chunkPtr)
        {
            std::cout << "ERROR: Attempted to send item pickup creation data for null chunk (" << itemPickupReference.chunk.x
                << ", " << itemPickupReference.chunk.y << ")\n";
            continue;
        }

        const ItemPickup* itemPickupPtr = chunkPtr->getItemPickup(itemPickupReference.id);
        if (!itemPickupPtr)
        {
            std::cout << "ERROR: Attempted to send item pickup creation data for null pickup ID " << itemPickupReference.id << "\n";
            continue;
        }

        ItemPickup itemPickup = *itemPickupPtr;

        // Normalise item pickup position relative to chunk before sending over network
        itemPickup.setPosition(itemPickup.getPosition() - chunkPtr->getWorldPosition());
        
        packetData.createdPickups.push_back({itemPickupReference, itemPickup});
    }

    if (packetData.createdPickups.size() <= 0)
    {
        return;
    }

    Packet packet;
    packet.set(packetData);
    networkHandler.sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
}

void Game::catchRandomFish(pl::Vector2<int> fishedTile)
{
    const BiomeGenData* biomeGenData = getChunkManager().getChunkBiome(ChunkPosition(fishedTile.x / CHUNK_TILE_SIZE, fishedTile.y / CHUNK_TILE_SIZE));

    // Check for nullptr
    if (!biomeGenData)
        return;

    std::vector<ItemPickupReference> itemPickupsCreatedVector;

    PacketDataItemPickupsCreateRequest packetData;
    packetData.locationState = locationState;
    
    // Randomise catch
    float randomChance = Helper::randInt(0, 10000) / 10000.0f;
    float cumulativeChance = 0.0f;
    for (const FishCatchData& fishCatchData : biomeGenData->fishCatchDatas)
    {
        cumulativeChance += fishCatchData.chance;
        
        if (cumulativeChance >= randomChance)
        {
            // Add fish / catch
            // inventory.addItem(fishCatchData.itemCatch, fishCatchData.count, true);

            // Create fish item pickups
            for (int i = 0; i < fishCatchData.count; i++)
            {
                pl::Vector2f spawnPos = player.getPosition() + pl::Vector2f(
                    Helper::randFloat(-TILE_SIZE_PIXELS_UNSCALED / 2.0f, TILE_SIZE_PIXELS_UNSCALED / 2.0f),
                    Helper::randFloat(-TILE_SIZE_PIXELS_UNSCALED / 2.0f, TILE_SIZE_PIXELS_UNSCALED / 2.0f)
                );

                if (networkHandler.isLobbyHostOrSolo())
                {
                    // Not multiplayer / is host, create pickups and tell clients
                    itemPickupsCreatedVector.push_back(getChunkManager().addItemPickup(ItemPickup(spawnPos, fishCatchData.itemCatch, gameTime)).value());
                }
                else
                {
                    // Multiplayer and is client, request pickup from host
                    ItemPickupRequest request;
                    request.chunk = WorldObject::getChunkInside(spawnPos, getChunkManager().getWorldSize());

                    // Get chunkPtr to calculate spawn relative position
                    // Cannot spawn if chunk does not exist, as cannot calculate relative position
                    Chunk* chunkPtr = getChunkManager().getChunk(request.chunk);
                    if (!chunkPtr)
                    {
                        break;
                    }

                    request.itemType = fishCatchData.itemCatch;
                    request.positionRelative = spawnPos - chunkPtr->getWorldPosition();
                    packetData.pickupRequests.push_back(request);
                }
            }

            break;
        }
    }

    if (!networkHandler.isMultiplayerGame())
    {
        return;
    }

    if (networkHandler.getIsLobbyHost())
    {
        itemPickupsCreated(itemPickupsCreatedVector, locationState);
    }
    else
    {
        Packet packet;
        packet.set(packetData, true);
        networkHandler.sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
    }
}

void Game::attemptObjectInteract()
{
    if (worldMenuState != WorldMenuState::Main && worldMenuState != WorldMenuState::Inventory)
    {
        return;
    }

    // Get mouse position in screen space and world space
    pl::Vector2f mouseWorldPos = camera.screenToWorldTransform(mouseScreenPos);

    if (!player.canReachPosition(mouseWorldPos))
        return;
    
    BuildableObject* selectedObject = getSelectedObjectFromChunkOrRoom();

    if (selectedObject)
    {
        selectedObject->interact(*this, networkHandler.isClient());
    }
}

void Game::attemptBuildObject()
{
    if (gameState != GameState::OnPlanet)
        return;
    
    if (player.isInRocket())
        return;

    ObjectType objectType = InventoryGUI::getHeldObjectType(inventory);

    if (objectType < 0)
        return;

    if (!player.canReachPosition(camera.screenToWorldTransform(mouseScreenPos)))
    {
        return;
    }

    if (!getChunkManager().canPlaceObject(Cursor::getSelectedChunk(getChunkManager().getWorldSize()), Cursor::getSelectedChunkTile(), objectType, player.getCollisionRect()))
    {
        return;
    }
    
    InventoryGUI::subtractHeldItem(inventory);

    buildObject(Cursor::getSelectedChunk(getChunkManager().getWorldSize()), Cursor::getSelectedChunkTile(), objectType);
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
    
    if (!getChunkManager().canPlaceLand(Cursor::getSelectedChunk(getChunkManager().getWorldSize()), Cursor::getSelectedChunkTile()))
        return;
    
    if (!player.canReachPosition(camera.screenToWorldTransform(mouseScreenPos)))
        return;
    
    // Place land
    getChunkManager().placeLand(Cursor::getSelectedChunk(getChunkManager().getWorldSize()), Cursor::getSelectedChunkTile());

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

    const PlanetGenData& planetGenData = PlanetGenDataLoader::getPlanetGenData(locationState.getPlanetType());
    const BiomeGenData* biomeGenData = Chunk::getBiomeGenAtWorldTile(player.getWorldTileInside(getChunkManager().getWorldSize()), getChunkManager().getWorldSize(),
        getChunkManager().getBiomeNoise(), locationState.getPlanetType());
    
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
    if (!getBossManager().createBoss(itemData.bossSummonData->bossName, player.getPosition(), *this))
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
    bool canPlace = getChunkManager().canPlaceObject(Cursor::getSelectedChunk(getChunkManager().getWorldSize()),
                                                Cursor::getSelectedChunkTile(),
                                                object,
                                                player.getCollisionRect());

    bool inRange = player.canReachPosition(camera.screenToWorldTransform(mouseScreenPos));

    pl::Color drawColor(255, 0, 0, 180);
    if (canPlace && inRange)
        drawColor = pl::Color(0, 255, 0, 180);
    
    BuildableObject objectGhost(Cursor::getLerpedSelectPos() + pl::Vector2f(TILE_SIZE_PIXELS_UNSCALED / 2.0f, TILE_SIZE_PIXELS_UNSCALED / 2.0f), object, false);

    objectGhost.draw(window, spriteBatch, *this, camera, 0.0f, 0, getChunkManager().getWorldSize(), drawColor);

    spriteBatch.endDrawing(window);
}

void Game::drawGhostPlaceLandAtCursor()
{
    pl::Vector2f tileWorldPosition = Cursor::getLerpedSelectPos();
    int worldSize = getChunkManager().getWorldSize();

    // Change color depending on whether can place land or not
    pl::Color landGhostColor(255, 0, 0, 180);
    if (getChunkManager().canPlaceLand(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile()) &&
        player.canReachPosition(camera.screenToWorldTransform(mouseScreenPos)))
    {
        landGhostColor = pl::Color(0, 255, 0, 180);
    }

    float scale = ResolutionHandler::getScale();

    // Sample noise to select correct tile to draw
    const BiomeGenData* biomeGenData = Chunk::getBiomeGenAtWorldTile(
        Cursor::getSelectedWorldTile(worldSize), worldSize, getChunkManager().getBiomeNoise(), locationState.getPlanetType()
        );
    
    // Check for nullptr (shouldn't happen)
    if (!biomeGenData)
        return;
    
    // Get texture offset for tilemap
    pl::Vector2<int> tileMapTextureOffset = biomeGenData->tileGenDatas[0].tileMap.textureOffset;

    // Create texture rect of centre tile from tilemap
    pl::Rect<int> textureRect(tileMapTextureOffset.x + 16, tileMapTextureOffset.y + 16, 16, 16);

    pl::DrawData drawData;
    drawData.texture = TextureManager::getTexture(TextureType::GroundTiles);
    drawData.shader = Shaders::getShader(ShaderType::Default);
    drawData.position = camera.worldToScreenTransform(tileWorldPosition);
    drawData.scale = pl::Vector2f(scale, scale);
    drawData.color = landGhostColor;
    drawData.textureRect = textureRect;

    // Draw tile at screen position
    spriteBatch.draw(window, drawData);
}

BuildableObject* Game::getSelectedObjectFromChunkOrRoom()
{
    pl::Vector2f mouseWorldPos = camera.screenToWorldTransform(mouseScreenPos);

    switch (gameState)
    {
        case GameState::OnPlanet:
        {
            return getObjectFromLocation(ObjectReference{Cursor::getSelectedChunk(getChunkManager().getWorldSize()), Cursor::getSelectedChunkTile()}, locationState);
        }
        case GameState::InStructure: // fallthrough
        case GameState::InRoomDestination:
        {
            return getObjectFromLocation(ObjectReference{{0, 0}, Cursor::getSelectedTile()}, locationState);
        }
    }

    return nullptr;
}

template <class T>
T* Game::getObjectFromLocation(ObjectReference objectReference, const LocationState& objectLocationState)
{
    switch (objectLocationState.getGameState())
    {
        case GameState::OnPlanet:
        {
            if (!worldDatas.contains(objectLocationState.getPlanetType()))
            {
                printf(("ERROR: Attempted to access object from null planet type " + std::to_string(objectLocationState.getPlanetType()) + "\n").c_str());
                break;
            }
            return getChunkManager(objectLocationState.getPlanetType()).getChunkObject<T>(objectReference.chunk, objectReference.tile);
        }
        case GameState::InStructure:
        {
            if (!worldDatas.contains(objectLocationState.getPlanetType()))
            {
                printf(("ERROR: Attempted to access object for structure from null planet type " + std::to_string(objectLocationState.getPlanetType()) + "\n").c_str());
                break;
            }
            if (!getStructureRoomPool(objectLocationState.getPlanetType()).isIDValid(objectLocationState.getInStructureID()))
            {
                printf(("ERROR: Attempted to access object from null structure ID " + std::to_string(objectLocationState.getInStructureID()) + "\n").c_str());
                break;
            }
            Room& structureRoom = getStructureRoomPool(objectLocationState.getPlanetType()).getRoom(objectLocationState.getInStructureID());
            return structureRoom.getObject<T>(objectReference.tile);
        }
        case GameState::InRoomDestination:
        {
            if (!roomDestDatas.contains(objectLocationState.getRoomDestType()))
            {
                printf(("ERROR: Attempted to access object from null room dest type " + std::to_string(objectLocationState.getRoomDestType()) + "\n").c_str());
                break;
            }
            return getRoomDestination(objectLocationState.getRoomDestType()).getObject<T>(objectReference.tile);
        }
    }

    return nullptr;
}


// -- Inventory / Chests -- //

void Game::handleInventoryClose()
{
    ItemType itemHeldBefore = InventoryGUI::getHeldItemType(inventory);

    InventoryGUI::handleClose(inventory, getChestDataPool().getChestDataPtr(openedChestID));
    worldMenuState = WorldMenuState::Main;
    closeChest();

    if (itemHeldBefore != InventoryGUI::getHeldItemType(inventory))
    {
        changePlayerTool();
    }   
}

void Game::openChest(ChestObject& chest, std::optional<LocationState> chestLocationState, bool initiatedClientSide)
{
    if (!chestLocationState.has_value())
    {
        chestLocationState = locationState;
    }

    // Initialise chest opened packet
    PacketDataChestOpened packetData;
    packetData.locationState = chestLocationState.value();
    if (chestLocationState->getGameState() == GameState::OnPlanet)
    {
        packetData.chestObject.chunk = chest.getChunkInside(getChunkManager().getWorldSize());
        packetData.chestObject.tile = chest.getChunkTileInside(getChunkManager().getWorldSize());
    }
    else
    {
        packetData.chestObject.tile = chest.getTileInside();
    }

    if (networkHandler.isMultiplayerGame())
    {
        packetData.userID = SteamUser()->GetSteamID().ConvertToUint64();
    }

    Packet packet;
    packet.set(packetData);

    if (initiatedClientSide && networkHandler.isClient())
    {
        // Alert server of attempt chest open
        networkHandler.sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
    }
    else
    {
        // Sent from host / is host / not multiplayer

        // Ensure chest is not open
        if (chest.isOpen())
        {
            return;
        }

        // Close open chest (if one is open)
        if (openedChestID != 0xFFFF)
        {
            closeChest();
        }

        // Open chest animation
        chest.openChest();

        if (networkHandler.getIsLobbyHost())
        {
            // If is host - tell clients chest has been opened
            networkHandler.sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
        }

        // If required
        InventoryGUI::shopClosed();
    
        openedChestID = chest.getChestID();

        openedChest = packetData.chestObject;
        // openedChestPos = chest.getPosition();
    
        InventoryGUI::chestOpened(getChestDataPool().getChestDataPtr(openedChestID));
    
        worldMenuState = WorldMenuState::Inventory;
    }
}

void Game::openChestForClient(PacketDataChestOpened packetData)
{
    if (!networkHandler.getIsLobbyHost())
    {
        return;
    }

    // ChestObject* chestObject = chunkManager.getChunkObject<ChestObject>(packetData.chestObject.chunk, packetData.chestObject.tile);
    ChestObject* chestObject = getObjectFromLocation<ChestObject>(packetData.chestObject, packetData.locationState);
    if (!chestObject)
    {
        return;
    }

    // If already open, do not allow client to open as well (prevents item sync issues)
    if (chestObject->isOpen())
    {
        return;
    }

    // Open chest host-side
    chestObject->openChest();
    
    // Initialise new chest if required
    packetData.chestID = chestObject->getChestID();
    if (packetData.chestID == 0xFFFF)
    {
        packetData.chestID = chestObject->createChestID(*this, packetData.locationState);
    }

    InventoryData* chestData = getChestDataPool(packetData.locationState).getChestDataPtr(packetData.chestID);

    // Failed to create new chest - abort
    if (!chestData)
    {
        return;
    }
    
    packetData.chestData = *chestData;
    Packet packet;
    packet.set(packetData, true);
    networkHandler.sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
}

void Game::openChestFromHost(const PacketDataChestOpened& packetData)
{
    if (!networkHandler.isClient())
    {
        return;
    }

    ChestObject* chestObject = getObjectFromLocation<ChestObject>(packetData.chestObject, packetData.locationState);

    if (!chestObject)
    {
        return;
    }

    // Opened by another user - make chest open with animation only
    if (packetData.userID != SteamUser()->GetSteamID().ConvertToUint64())
    {
        chestObject->triggerBehaviour(*this, ObjectBehaviourTrigger::ChestOpen);
        return;
    }

    // Set chest ID and data, and open chest
    chestObject->setChestID(packetData.chestID);
    getChestDataPool().overwriteChestData(packetData.chestID, packetData.chestData);
    chestObject->interact(*this, false); // isClient is passed as false as now has server auth to open chest
}

void Game::openedChestDataModified()
{
    // Only send packet if not lobby host (is client), as host updates chest data for client on open regardless
    if (!networkHandler.isClient() || getChestDataPool().getChestDataPtr(openedChestID) == nullptr)
    {
        return;
    }

    PacketDataChestDataModified packetData;
    packetData.chestID = openedChestID;
    packetData.chestData = *getChestDataPool().getChestDataPtr(openedChestID);
    packetData.locationState = locationState;
    Packet packet;
    packet.set(packetData, true);
    
    printf("Sending chest data to host\n");

    networkHandler.sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
}

uint16_t Game::getOpenChestID()
{
    return openedChestID;
}

void Game::checkChestOpenInRange()
{
    if (openedChestID == 0xFFFF)
        return;
    
    BuildableObject* chestObject = getObjectFromLocation(openedChest, locationState);
    if (!chestObject)
    {
        closeChest();
        return;
    }

    if (!player.canReachPosition(chestObject->getPosition()))
    {
        closeChest();
    }
}

void Game::closeChest(std::optional<ObjectReference> chestObjectRef, std::optional<LocationState> chestLocationState, bool sentFromHost, std::optional<uint64_t> userId)
{
    if (!chestObjectRef.has_value())
    {
        // Default chest to this player's currently open chest
        chestObjectRef = openedChest;

        // If this player does not have a chest open (and has defaulted to this player), then do not attempt to close chest
        if (openedChestID == 0xFFFF)
        {
            return;
        }
    }

    if (!chestLocationState.has_value())
    {
        chestLocationState = locationState;
    }

    // Networking for chest
    if (networkHandler.isMultiplayerGame())
    {
        PacketDataChestClosed packetData;
        packetData.locationState = chestLocationState.value();
        packetData.chestObject = chestObjectRef.value();
        packetData.userID = SteamUser()->GetSteamID().ConvertToUint64();
        Packet packet;
        packet.set(packetData);
        if (networkHandler.isClient() && !sentFromHost)
        {
            // Alert host of chest close
            networkHandler.sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
        }
        else if (networkHandler.getIsLobbyHost())
        {
            // Alert clients
            networkHandler.sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);   
        }
    }

    // Close chest
    BuildableObject* object = getObjectFromLocation(chestObjectRef.value(), chestLocationState.value());
    if (object)
    {
        object->triggerBehaviour(*this, ObjectBehaviourTrigger::ChestClose);
    }
    else
    {
        printf("ERROR: Attempted to close null chest\n");
    }

    // If sent from host or is user (this client triggered this close so UI already closed), do not close UI
    bool isUser = false;
    if (userId.has_value() && steamInitialised)
    {
        isUser = (userId.value() == SteamUser()->GetSteamID().ConvertToUint64());
    }

    if (sentFromHost || isUser)
    {
        return;
    }

    // Chest closed is not currently opened chest - do not close UI
    if (chestObjectRef.value() != openedChest)
    {
        return;
    }

    InventoryGUI::chestClosed();

    openedChestID = 0xFFFF;
    openedChest.chunk = ChunkPosition(0, 0);
    openedChest.tile = pl::Vector2<int>(0, 0);
    // openedChestPos = pl::Vector2f(0, 0);
}


// -- Planet travelling -- //

void Game::travelToDestination()
{
    travelTrigger = false;

    // If client, request travel from host
    if (networkHandler.isClient())
    {
        Packet packet;
        if (destinationLocationState.isOnPlanet())
        {
            PacketDataPlanetTravelRequest packetData;
            packetData.planetType = destinationLocationState.getPlanetType();
            packetData.rocketUsedReference = rocketEnteredReference;
            packet.set(packetData);
        }
        else if (destinationLocationState.isInRoomDest())
        {
            // TODO: Room travel request
        }

        networkHandler.sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
        return;
    }

    LocationState previousLocationState = locationState;
    
    player.exitRocket();

    if (destinationLocationState.isOnPlanet())
    {
        ObjectReference newRocket = setupPlanetTravel(destinationLocationState.getPlanetType(), locationState, rocketEnteredReference, std::nullopt);
        // deleteUsedRocket(rocketEnteredReference, locationState.getPlanetType());
        travelToPlanet(destinationLocationState.getPlanetType(), newRocket);
    }
    else if (destinationLocationState.isInRoomDest())
    {
        travelToRoomDestination(destinationLocationState.getRoomDestType());
    }

    // Only clear other world / room dest datas if not multiplayer / is client
    if (!networkHandler.getIsLobbyHost())
    {
        if (previousLocationState.isOnPlanet())
        {
            worldDatas.erase(previousLocationState.getPlanetType());
        }
        if (previousLocationState.isInRoomDest())
        {
            roomDestDatas.erase(previousLocationState.getRoomDestType());
        }
    }

    particleSystem.clear();
    nearbyCraftingStationLevels.clear();

    destinationLocationState.setToNull();
}

void Game::deleteObjectSynced(ObjectReference objectReference, PlanetType planetType)
{
    if (!networkHandler.isLobbyHostOrSolo() || planetType < 0)
    {
        return;
    }

    if (!worldDatas.contains(planetType))
    {
        printf(("WARNING: Cannot delete object for null planet " + std::to_string(planetType) + "\n").c_str());
        return;
    }

    getChunkManager(planetType).deleteObject(objectReference.chunk, objectReference.tile);

    // Alert clients if required
    if (!networkHandler.isMultiplayerGame())
    {
        return;
    }

    PacketDataObjectDestroyed packetData;
    packetData.planetType = planetType;
    packetData.objectReference = objectReference;
    packetData.userId = SteamUser()->GetSteamID().ConvertToUint64();
    
    Packet packet;
    packet.set(packetData);
    networkHandler.sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
}

ObjectReference Game::setupPlanetTravel(PlanetType planetType, const LocationState& currentLocation, ObjectReference rocketObjectUsed, std::optional<uint64_t> clientID)
{
    assert(networkHandler.isLobbyHostOrSolo());

    if (!worldDatas.contains(planetType))
    {
        loadPlanet(planetType);
    }

    // Get used rocket object data
    RocketObject* rocketObject = getObjectFromLocation<RocketObject>(rocketObjectUsed, currentLocation);
    if (!rocketObject)
    {
        printf("ERROR: Attempted to set up planet travel for null rocket object (%d, %d, %d, %d)\n",
            rocketObjectUsed.chunk.x, rocketObjectUsed.chunk.y, rocketObjectUsed.tile.x, rocketObjectUsed.tile.y);
        return {{0, 0}, {0, 0}};
    }

    ObjectType rocketObjectType = rocketObject->getObjectType();

    // Delete used rocket object if on planet
    if (currentLocation.getGameState() == GameState::OnPlanet)
    {
        deleteObjectSynced(rocketObjectUsed, currentLocation.getPlanetType());
    }

    const std::unordered_map<PlanetType, ObjectReference>* planetRocketsUsedPtr = &planetRocketUsedPositions;
    if (clientID.has_value())
    {
        planetRocketsUsedPtr = &networkHandler.getSavedNetworkPlayerData(clientID.value())->planetRocketUsedPositions;
    }

    // Get rocket spawn for player
    assert(worldDatas.contains(planetType));

    ObjectReference placeRocketReference;

    if (!planetRocketsUsedPtr->contains(planetType))
    {
        // Create rocket at default spawn location
        ChunkPosition rocketChunk = getChunkManager(planetType).findValidSpawnChunk(2);
        placeRocketReference = ObjectReference{rocketChunk, pl::Vector2<int>(0, 0)};
    }
    else
    {
        // Create rocket at previously used rocket location
        placeRocketReference = planetRocketsUsedPtr->at(planetType);
    }

    ChunkViewRange playerChunkViewRange = camera.getChunkViewRange();
    if (clientID.has_value())
    {
        playerChunkViewRange = networkHandler.getNetworkPlayer(clientID.value())->getChunkViewRange();
    }

    playerChunkViewRange = playerChunkViewRange.copyAndCentre(placeRocketReference.chunk);
    
    // Update chunks at rocket position
    getChunkManager(planetType).updateChunks(*this, std::nullopt, {playerChunkViewRange});
    
    // Place rocket
    buildObject(placeRocketReference.chunk, placeRocketReference.tile, rocketObjectType, planetType, false, false);

    // Send chunks of new planet to client if client is travelling
    if (clientID.has_value() && networkHandler.getIsLobbyHost())
    {
        // Send chunks to client
        PacketDataPlanetTravelReply packetData;
        packetData.chunkDatas.planetType = planetType;
        packetData.rocketObjectReference = placeRocketReference;

        for (auto iter = playerChunkViewRange.begin(); iter != playerChunkViewRange.end(); iter++)
        {
            ChunkPosition chunkPos = iter.get(getChunkManager(planetType).getWorldSize());
            packetData.chunkDatas.chunkDatas.push_back(getChunkManager(planetType).getChunkDataAndGenerate(chunkPos, *this));
        }

        printf(("PLANET TRAVEL: Sending planet travel data to client for planet type " + std::to_string(planetType) + "\n").c_str());

        Packet packet;
        packet.set(packetData, true);
        networkHandler.sendPacketToClient(clientID.value(), packet, k_nSteamNetworkingSend_Reliable, 0);
    }

    return placeRocketReference;
}

void Game::travelToPlanet(PlanetType planetType, ObjectReference newRocketObjectReference)
{
    // Store rocket used from previous planet in map
    if (locationState.isOnPlanet())
    {
        planetRocketUsedPositions[locationState.getPlanetType()] = rocketEnteredReference;
    }

    locationState.setPlanetType(planetType);
    
    overrideState(GameState::OnPlanet);

    camera.instantUpdate(player.getPosition());
    
    if (gameState == GameState::OnPlanet && networkHandler.isLobbyHostOrSolo())
    {
        getChunkManager().updateChunks(*this, player.getPosition(), {camera.getChunkViewRange()});
    }
    lightingTick = LIGHTING_TICK;

    rocketEnteredReference = newRocketObjectReference;
    
    // Start rocket flying downwards
    RocketObject* rocketObject = getObjectFromLocation<RocketObject>(rocketEnteredReference, locationState);
    if (rocketObject)
    {
        // TODO: Find empty tile around rocket
        player.setPosition(rocketObject->getPosition() - pl::Vector2f(1, 1) * TILE_SIZE_PIXELS_UNSCALED);
        rocketObject->triggerBehaviour(*this, ObjectBehaviourTrigger::RocketFlyDown);
    }
    else
    {
        printf("ERROR: Cannot enter null rocket when travelling to planet\n");
    }
    
    camera.instantUpdate(player.getPosition());

    networkHandler.sendPlayerData();
}

void Game::travelToPlanetFromHost(const PacketDataPlanetTravelReply& planetTravelReplyPacket)
{
    if (!networkHandler.isClient())
    {
        return;
    }

    particleSystem.clear();
    nearbyCraftingStationLevels.clear();

    destinationLocationState.setToNull();

    worldDatas.clear();
    roomDestDatas.clear();

    initialiseWorldData(planetTravelReplyPacket.chunkDatas.planetType);

    for (const auto& chunkData : planetTravelReplyPacket.chunkDatas.chunkDatas)
    {
        getChunkManager(planetTravelReplyPacket.chunkDatas.planetType).setChunkData(chunkData, *this);
    }

    player.exitRocket();

    travelToPlanet(planetTravelReplyPacket.chunkDatas.planetType, planetTravelReplyPacket.rocketObjectReference);
}

void Game::travelToRoomDestination(RoomType destinationRoomType)
{
    overrideState(GameState::InRoomDestination);
    
    // roomDestinationManager.loadRoomDestinationType(destinationRoom, chestDataPool);
    GameSaveIO io(currentSaveFileSummary.name);

    RoomDestinationGameSave roomDestinationGameSave;

    locationState.setRoomDestType(destinationRoomType);

    roomDestDatas[locationState.getRoomDestType()] = RoomDestinationData();
    
    if (io.loadRoomDestinationSave(destinationRoomType, roomDestinationGameSave))
    {
        getChestDataPool(locationState) = roomDestinationGameSave.chestDataPool;
        getRoomDestination(locationState.getRoomDestType()) = roomDestinationGameSave.roomDestination;
    }
    else
    {
        getChestDataPool() = ChestDataPool();
        getRoomDestination() = Room(destinationRoomType, &getChestDataPool());
    }

    if (getRoomDestination().getFirstRocketObjectReference(rocketEnteredReference))
    {
        BuildableObject* rocketObject = getObjectFromLocation(rocketEnteredReference, locationState);

        if (rocketObject)
        {
            player.setPosition(rocketObject->getPosition() - pl::Vector2f(TILE_SIZE_PIXELS_UNSCALED, 0));

            rocketObject->triggerBehaviour(*this, ObjectBehaviourTrigger::RocketFlyDown);
        }
    }
    else
    {
        std::cout << "Error: could not find rocket object in room destination\n";
    }

    camera.instantUpdate(player.getPosition());
}

ChunkPosition Game::initialiseNewPlanet(PlanetType planetType)
{
    initialiseWorldData(planetType);

    ChunkPosition playerSpawnChunk = getChunkManager(planetType).findValidSpawnChunk(2);

    // pl::Vector2f playerSpawnPos;
    // playerSpawnPos.x = playerSpawnChunk.x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    // playerSpawnPos.y = playerSpawnChunk.y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    // player.setPosition(playerSpawnPos);

    // camera.instantUpdate(player.getPosition());

    // if (networkHandler.isLobbyHostOrSolo())
    // {
    //     getChunkManager(planetType).updateChunks(*this, camera);
    // }

    // weatherSystem = WeatherSystem(gameTime, getChunkManager(planetType).getSeed() + planetType);
    // weatherSystem.presimulateWeather(gameTime, camera, getChunkManager(planetType));

    // Ensure spawn chunk does not have structure
    getChunkManager(planetType).regenerateChunkWithoutStructure(playerSpawnChunk, *this);

    return playerSpawnChunk;

    // // Place rocket
    // if (placeRocket)
    // {
    //     getChunkManager(planetType).setObject(playerSpawnChunk, pl::Vector2<int>(0, 0), ObjectDataLoader::getObjectTypeFromName("Rocket Launch Pad"), *this);
    //     rocketEnteredReference.chunk = playerSpawnChunk;
    //     rocketEnteredReference.tile = pl::Vector2<int>(0, 0);
    // }
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
    float alpha = 1.0f - transitionGameStateTimer / TRANSITION_STATE_FADE_TIME;
    
    pl::VertexArray fadeRect;
    fadeRect.addQuad(pl::Rect<float>(0, 0, window.getWidth(), window.getHeight()), pl::Color(0, 0, 0, 255 * alpha), pl::Rect<float>());

    window.draw(fadeRect, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);
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
        case GameState::MainMenu:
        {
            ResolutionHandler::overrideZoom(0);
            worldDatas.clear();
            roomDestDatas.clear();
            locationState.setToNull();
            break;
        }
        case GameState::InStructure:
        {
            closeChest();
            nearbyCraftingStationLevels.clear();
            
            if (gameState == GameState::OnPlanet)
            {
                std::optional<pl::Vector2f> roomEntrancePos = getStructureRoomPool().getRoom(locationState.getInStructureID()).getEntrancePosition();

                //assert(roomEntrancePos.has_value());
                if (!roomEntrancePos.has_value())
                {
                    roomEntrancePos = pl::Vector2f(50, 50);
                }

                player.setPosition(roomEntrancePos.value());
            }

            camera.instantUpdate(player.getPosition());

            player.enterStructure();

            networkHandler.sendPlayerData();
            break;
        }
        case GameState::OnPlanet:
        {
            if (gameState == GameState::InStructure)
            {
                // Exit structure
                locationState.setInStructureID(std::nullopt);

                player.setPosition(structureEnteredPos);
                camera.instantUpdate(player.getPosition());
            }

            networkHandler.sendPlayerData();
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
    networkHandler.reset(this);

    player = Player(pl::Vector2f(0, 0));
    inventory = InventoryData(32);
    inventory.giveStartingItems();
    armourInventory = InventoryData(3);
    InventoryGUI::reset();
    changePlayerTool();

    locationState = LocationState();
    locationState.setPlanetType(PlanetGenDataLoader::getPlanetTypeFromName("Earthlike"));
    planetSeed = seed;

    worldDatas.clear();
    roomDestDatas.clear();
    worldDatas[locationState.getPlanetType()] = WorldData();

    ChunkPosition spawnChunk = initialiseNewPlanet(locationState.getPlanetType());

    player.setPosition(pl::Vector2f(spawnChunk.x + 0.5f, spawnChunk.y + 0.5f) * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED);
    
    dayCycleManager.setCurrentTime(dayCycleManager.getDayLength() * 0.5f);
    dayCycleManager.setCurrentDay(1);

    saveSessionPlayTime = 0.0f;

    getBossManager().clearBosses();
    getProjectileManager().clear();
    // enemyProjectileManager.clear();
    getLandmarkManager().clear();

    gameTime = 0.0f;

    weatherSystem = WeatherSystem(gameTime, seed + locationState.getPlanetType());
    weatherSystem.presimulateWeather(gameTime, camera, getChunkManager());

    camera.instantUpdate(player.getPosition());

    getChunkManager().updateChunks(*this, player.getPosition(), {camera.getChunkViewRange()});
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
    playerGameSave.seed = planetSeed;
    playerGameSave.time = dayCycleManager.getCurrentTime();
    playerGameSave.day = dayCycleManager.getCurrentDay();

    // Add this player data to save
    playerGameSave.playerData = createPlayerData();

    playerGameSave.networkPlayerDatas = networkHandler.getSavedNetworkPlayerDataMap();

    // Keep track of which planets / room dests require saving
    std::unordered_set<PlanetType> planetTypesToSave = networkHandler.getPlayersPlanetTypeSet(locationState.getPlanetType());
    std::unordered_set<RoomType> roomDestsToSave = networkHandler.getPlayersRoomDestTypeSet(locationState.getRoomDestType());

    // Add play time
    currentSaveFileSummary.timePlayed += std::round(saveSessionPlayTime);
    saveSessionPlayTime = 0.0f;
    playerGameSave.timePlayed = currentSaveFileSummary.timePlayed;
    
    io.writePlayerSave(playerGameSave);

    // Save planets required
    for (PlanetType planetType : planetTypesToSave)
    {
        PlanetGameSave planetGameSave;
        planetGameSave.chunks = getChunkManager(planetType).getChunkPODs();
        planetGameSave.chestDataPool = getChestDataPool(LocationState::createFromPlanetType(planetType));
        planetGameSave.structureRoomPool = getStructureRoomPool(planetType);
        io.writePlanetSave(planetType, planetGameSave);
    }

    // Save room dests required
    for (RoomType roomDestType : roomDestsToSave)
    {
        RoomDestinationGameSave roomDestGameSave;
        roomDestGameSave.roomDestination = getRoomDestination(roomDestType);
        roomDestGameSave.chestDataPool = getChestDataPool(LocationState::createFromRoomDestType(roomDestType));
        io.writeRoomDestinationSave(roomDestGameSave);
    }
    
    // PlanetGameSave planetGameSave;
    // RoomDestinationGameSave roomDestinationGameSave;

    // switch (gameState)
    // {
    //     // case GameState::InStructure:
    //     // {
    //     //     planetGameSave.isInRoom = true;
    //     //     planetGameSave.inRoomID = structureEnteredID;
    //     //     planetGameSave.positionInRoom = player.getPosition();

    //     //     planetGameSave.playerLastPlanetPos = structureEnteredPos;
    //     // } // fallthrough
    //     case GameState::OnPlanet:
    //     {
    //         planetGameSave.chunks = chunkManager.getChunkPODs();
    //         planetGameSave.chestDataPool = chestDataPool;
    //         planetGameSave.structureRoomPool = structureRoomPool;
            
    //         playerGameSave.planetType = chunkManager.getPlanetType();
    //         planetGameSave.playerLastPlanetPos = player.getPosition();
            
    //         if (gettingInRocket)
    //         {
    //             planetGameSave.rocketObjectUsed = rocketEnteredReference;
    //         }
    //         break;
    //     }
    //     case GameState::InRoomDestination:
    //     {
    //         roomDestinationGameSave.roomDestination = roomDestination;
    //         roomDestinationGameSave.chestDataPool = chestDataPool;
    //         roomDestinationGameSave.playerLastPos = player.getPosition();

    //         playerGameSave.roomDestinationType = roomDestination.getRoomType();
    //         break;
    //     }
    // }


    // switch (gameState)
    // {
    //     case GameState::InStructure: // fallthrough
    //     case GameState::OnPlanet:
    //     {
    //         io.writePlanetSave(playerGameSave.planetType, planetGameSave);
    //         break;
    //     }
    //     case GameState::InRoomDestination:
    //     {
    //         io.writeRoomDestinationSave(roomDestinationGameSave);
    //         break;
    //     }
    // }

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

    networkHandler.reset(this);
    worldDatas.clear();
    roomDestDatas.clear();

    player = Player(playerGameSave.playerData.position, playerGameSave.playerData.maxHealth);

    inventory = playerGameSave.playerData.inventory;
    armourInventory = playerGameSave.playerData.armourInventory;
    
    InventoryGUI::reset();
    InventoryGUI::setSeenRecipes(playerGameSave.playerData.recipesSeen);

    planetRocketUsedPositions = playerGameSave.playerData.planetRocketUsedPositions;

    closeChest();
    
    // chunkManager.setSeed(playerGameSave.seed);
    // inventory = playerGameSave.inventory;
    // armourInventory = playerGameSave.armourInventory;
    planetSeed = playerGameSave.seed;
    dayCycleManager.setCurrentTime(playerGameSave.time);
    dayCycleManager.setCurrentDay(playerGameSave.day);

    changePlayerTool();

    GameState nextGameState = GameState::OnPlanet;
    worldMenuState = WorldMenuState::Main;
    musicGap = MUSIC_GAP_MIN;

    // landmarkManager.clear();
        
    // Sync time-based systems, e.g. weather
    gameTime = playerGameSave.timePlayed;

    locationState = playerGameSave.playerData.locationState;

    // Load player datas for all network player saves
    for (auto iter = playerGameSave.networkPlayerDatas.begin(); iter != playerGameSave.networkPlayerDatas.end(); iter++)
    {
        networkHandler.setSavedNetworkPlayerData(iter->first, iter->second);
    }

    // Load planet
    if (playerGameSave.playerData.locationState.isOnPlanet())
    {
        PlanetGameSave planetGameSave;

        if (!io.loadPlanetSave(playerGameSave.playerData.locationState.getPlanetType(), planetGameSave))
        {
            const std::string& planetName = PlanetGenDataLoader::getPlanetGenData(playerGameSave.playerData.locationState.getPlanetType()).name;
            std::cout << "Failed to load planet \"" + planetName + "\" for save " + saveFileSummary.name + "\n";
            return false;
        }

        initialiseWorldData(locationState.getPlanetType());
        getChunkManager().loadFromChunkPODs(planetGameSave.chunks, *this);
        getChestDataPool() = planetGameSave.chestDataPool;
        getStructureRoomPool() = planetGameSave.structureRoomPool;

        // chunkManager.setPlanetType(playerGameSave.planetType);
        // chunkManager.loadFromChunkPODs(planetGameSave.chunks, *this);
        // chestDataPool = planetGameSave.chestDataPool;
        // structureRoomPool = planetGameSave.structureRoomPool;

        nextGameState = GameState::OnPlanet;

        camera.instantUpdate(player.getPosition());
        
        if (playerGameSave.playerData.locationState.isInStructure())
        {
            structureEnteredPos = playerGameSave.playerData.structureExitPos;
            
            nextGameState = GameState::InStructure;

            // Simulate weather outside of room
            camera.instantUpdate(structureEnteredPos);
        }
        
        weatherSystem = WeatherSystem(gameTime, planetSeed + locationState.getPlanetType());
        weatherSystem.presimulateWeather(gameTime, camera, getChunkManager());
        
        getChunkManager().updateChunks(*this, player.getPosition(), {camera.getChunkViewRange()});

        lightingTick = LIGHTING_TICK;
    }
    else if (playerGameSave.playerData.locationState.isInRoomDest())
    {
        // Load room destination
        RoomDestinationGameSave roomDestinationGameSave;

        if (!io.loadRoomDestinationSave(playerGameSave.playerData.locationState.getRoomDestType(), roomDestinationGameSave))
        {
            const std::string& roomDestinationName = StructureDataLoader::getRoomData(playerGameSave.playerData.locationState.getRoomDestType()).name;
            std::cout << "Failed to load room \"" + roomDestinationName + "\" for save " + saveFileSummary.name + "\n";
            return false;
        }

        roomDestDatas[locationState.getRoomDestType()] = RoomDestinationData();
        getRoomDestination() = roomDestinationGameSave.roomDestination;
        getChestDataPool() = roomDestinationGameSave.chestDataPool;

        // player.setPosition(roomDestinationGameSave.playerLastPos);

        nextGameState = GameState::InRoomDestination;
    }

    // bossManager.clearBosses();
    // projectileManager.clear();
    // enemyProjectileManager.clear();
    // particleSystem.clear();

    camera.instantUpdate(player.getPosition());

    // Load successful, set save name as current save and start state transition
    currentSaveFileSummary = saveFileSummary;
    currentSaveFileSummary.playerName = playerGameSave.playerData.name;
    startChangeStateTransition(nextGameState);

    saveSessionPlayTime = 0.0f;

    // Fade out previous music
    Sounds::stopMusic(0.3f);

    return true;
}

bool Game::loadPlanet(PlanetType planetType)
{
    if (worldDatas.contains(planetType))
    {
        return false;
    }

    GameSaveIO io(currentSaveFileSummary.name);

    PlanetGameSave planetGameSave;
    
    if (!io.loadPlanetSave(planetType, planetGameSave))
    {
        initialiseNewPlanet(planetType);
        return false;
    }

    initialiseWorldData(planetType);
    
    // if (planetGameSave.isInRoom)
    // {
    //     overrideState(GameState::InStructure);
    //     player.setPosition(planetGameSave.positionInRoom);
    //     structureEnteredID = planetGameSave.inRoomID;
    //     structureEnteredPos = planetGameSave.playerLastPlanetPos;
        
    //     // Simulate weather outside of room
    //     camera.instantUpdate(structureEnteredPos);
    // }
    // else
    // {
    //     overrideState(GameState::OnPlanet);
    //     player.setPosition(planetGameSave.playerLastPlanetPos);
        
    //     camera.instantUpdate(player.getPosition());
    // }
    
    // weatherSystem = WeatherSystem(gameTime, getChunkManager(planetType).getSeed() + planetType);
    // weatherSystem.presimulateWeather(gameTime, camera, getChunkManager(planetType));

    // camera.instantUpdate(player.getPosition());

    getChunkManager(planetType).loadFromChunkPODs(planetGameSave.chunks, *this);
    getChestDataPool(LocationState::createFromPlanetType(planetType)) = planetGameSave.chestDataPool;
    getStructureRoomPool(planetType) = planetGameSave.structureRoomPool;

    // assert(planetGameSave.rocketObjectUsed.has_value());

    // if (planetGameSave.rocketObjectUsed.has_value())
    // {
    //     rocketEnteredReference = planetGameSave.rocketObjectUsed.value();
    // }
    // else
    // {
    //     return false;
    // }

    // rocketObject = planetGameSave.rocketObjectUsed.value();

    return true;
}

PlayerData Game::createPlayerData()
{
    PlayerData playerData;

    playerData.name = currentSaveFileSummary.playerName;
    playerData.position = player.getPosition();
    playerData.inventory = inventory;
    playerData.armourInventory = armourInventory;
    playerData.maxHealth = player.getMaxHealth();
    
    playerData.locationState = locationState;
    playerData.structureExitPos = structureEnteredPos;

    playerData.recipesSeen = InventoryGUI::getSeenRecipes();

    playerData.planetRocketUsedPositions = planetRocketUsedPositions;

    return playerData;
}

void Game::initialiseWorldData(PlanetType planetType)
{
    if (worldDatas.contains(planetType))
    {
        printf(("WARNING: Initialising pre-existing world data for planet type " + std::to_string(planetType) + "\n").c_str());
    }

    worldDatas[planetType] = WorldData();
    getChunkManager(planetType).setSeed(planetSeed);
    getChunkManager(planetType).setPlanetType(planetType);
}

void Game::saveOptions()
{
    OptionsSave optionsSave;
    optionsSave.musicVolume = Sounds::getMusicVolume();
    optionsSave.screenShakeEnabled = Camera::getScreenShakeEnabled();
    optionsSave.controllerGlyphType = InputManager::getGlyphType();
    optionsSave.vSync = ResolutionHandler::getVSync();

    GameSaveIO optionsIO;
    optionsIO.writeOptionsSave(optionsSave);
}

void Game::loadOptions()
{
    OptionsSave optionsSave;

    GameSaveIO optionsIO;
    optionsIO.loadOptionsSave(optionsSave);

    Sounds::setMusicVolume(optionsSave.musicVolume);
    Camera::setScreenShakeEnabled(optionsSave.screenShakeEnabled);
    InputManager::setGlyphType(optionsSave.controllerGlyphType);
    ResolutionHandler::setVSync(optionsSave.vSync);
}

void Game::quitWorld()
{
    if (networkHandler.isLobbyHostOrSolo())
    {
        saveGame();
    }

    if (networkHandler.isMultiplayerGame())
    {
        networkHandler.leaveLobby();
        networkHandler.reset(this);
    }

    currentSaveFileSummary.name = "";
    startChangeStateTransition(GameState::MainMenu);
    mainMenuGUI.initialise();
    Sounds::stopMusic();
}


// -- Multiplayer --

void Game::joinWorld(const PacketDataJoinInfo& joinInfo)
{
    player = Player(joinInfo.playerData.position, joinInfo.playerData.maxHealth);
    inventory = joinInfo.playerData.inventory;
    armourInventory = joinInfo.playerData.armourInventory;

    currentSaveFileSummary.playerName = joinInfo.playerData.name;

    InventoryGUI::reset();
    InventoryGUI::setSeenRecipes(joinInfo.playerData.recipesSeen);

    changePlayerTool();

    planetSeed = joinInfo.seed;

    // player.setPosition(joinInfo.spawnPosition);
    camera.instantUpdate(player.getPosition());

    worldDatas.clear();
    roomDestDatas.clear();

    planetRocketUsedPositions = joinInfo.playerData.planetRocketUsedPositions;

    GameState nextGameState = GameState::OnPlanet;

    locationState = joinInfo.playerData.locationState;

    if (locationState.isOnPlanet())
    {
        initialiseWorldData(locationState.getPlanetType());
        
        if (locationState.isInStructure())
        {
            nextGameState = GameState::InStructure;
            structureEnteredPos = joinInfo.playerData.structureExitPos;
        }

        weatherSystem = WeatherSystem(gameTime, planetSeed + locationState.getPlanetType());
        weatherSystem.presimulateWeather(gameTime, camera, getChunkManager());
    }
    else if (locationState.isInRoomDest())
    {
        nextGameState = GameState::InRoomDestination;
        roomDestDatas[locationState.getRoomDestType()] = RoomDestinationData();
    }

    // chunkManager.setSeed(joinInfo.seed);
    // chunkManager.setPlanetType(PlanetGenDataLoader::getPlanetTypeFromName(joinInfo.planetName));s

    // chestDataPool = joinInfo.chestDataPool;
    // structureRoomPool = RoomPool();
    // landmarkManager = LandmarkManager();
    // particleSystem.clear();

    dayCycleManager.setCurrentTime(joinInfo.time);
    dayCycleManager.setCurrentDay(joinInfo.day);

    gameTime = joinInfo.gameTime;

    lightingTick = LIGHTING_TICK;

    // Send player data to host
    networkHandler.sendPlayerData();

    worldMenuState = WorldMenuState::Main;
    musicGap = MUSIC_GAP_MIN;
    startChangeStateTransition(nextGameState);
}

void Game::handleChunkRequestsFromClient(const PacketDataChunkRequests& chunkRequests, const SteamNetworkingIdentity& client)
{
    PacketDataChunkDatas packetChunkDatas;

    const char* steamName = SteamFriends()->GetFriendPersonaName(client.GetSteamID());
    
    int minChunkX = 9999999;
    int minChunkY = 9999999;
    int maxChunkX = -9999999;
    int maxChunkY = -9999999;

    // Get planet type for client
    if (chunkRequests.planetType < 0)
    {
        return;
    }

    if (!worldDatas.contains(chunkRequests.planetType))
    {
        printf(("ERROR: Attempted to send chunks to client for uninitialised planet type " + std::to_string(chunkRequests.planetType) + "\n").c_str());
        return;
    }

    packetChunkDatas.planetType = chunkRequests.planetType;

    for (ChunkPosition chunk : chunkRequests.chunkRequests)
    {
        packetChunkDatas.chunkDatas.push_back(getChunkManager(chunkRequests.planetType).getChunkDataAndGenerate(chunk, *this));
        minChunkX = std::min((int)chunk.x, minChunkX);
        minChunkY = std::min((int)chunk.y, minChunkY);
        maxChunkX = std::max((int)chunk.x, maxChunkX);
        maxChunkY = std::max((int)chunk.y, maxChunkY);
    }

    const PlanetGenData& planetData = PlanetGenDataLoader::getPlanetGenData(chunkRequests.planetType);

    Packet packet;
    packet.set(packetChunkDatas, true);
    
    printf(("NETWORK: (\"" + planetData.name + "\") Sending " + std::to_string(chunkRequests.chunkRequests.size()) + " chunks in range (" + std::to_string(minChunkX) +
        ", " + std::to_string(minChunkY) + ") to (" + std::to_string(maxChunkX) + ", " + std::to_string(maxChunkY) + ") to " + steamName + " " + packet.getSizeStr() +
        "\n").c_str());

    networkHandler.sendPacketToClient(client.GetSteamID64(), packet, k_nSteamNetworkingSend_Reliable, 0);
}

void Game::handleChunkDataFromHost(const PacketDataChunkDatas& chunkDataPacket)
{
    if (chunkDataPacket.planetType != locationState.getPlanetType() || !locationState.isOnPlanet())
    {
        printf(("ERROR: Received chunks from host for incorrect planet type " + std::to_string(chunkDataPacket.planetType) + "\n").c_str());
    }

    for (const auto& chunkData : chunkDataPacket.chunkDatas)
    {
        getChunkManager().setChunkData(chunkData, *this);

        printf(("NETWORK: Received chunk (" + std::to_string(chunkData.chunkPosition.x) + ", " +
            std::to_string(chunkData.chunkPosition.y) + ") data from host\n").c_str());
    }

    // Chunk datas received - recalcute lighting
    lightingTick = LIGHTING_TICK;
}

void Game::joinedLobby(bool requiresNameInput)
{
    if (gameState != GameState::MainMenu)
    {
        networkHandler.leaveLobby();
        return;
    }
    
    if (requiresNameInput)
    {
        mainMenuGUI.setMainMenuJoinGame();
    }
    else
    {
        networkHandler.sendWorldJoinReply();
    }
}


// -- Window -- //

void Game::handleZoom(int zoomChange)
{
    float beforeScale = ResolutionHandler::getScale();
    ResolutionHandler::changeZoom(zoomChange);
    
    float afterScale = ResolutionHandler::getScale();

    camera.handleScaleChange(beforeScale, afterScale, player.getPosition());

    // Recalculate lighting
    lightingTick = LIGHTING_TICK;
}

void Game::handleEventsWindow(const SDL_Event& event)
{
    if (event.type == SDL_WINDOWEVENT)
    {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
            handleWindowResize(pl::Vector2<uint32_t>(event.window.data1, event.window.data2));
            return;
        }
    }

    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.scancode == SDL_SCANCODE_F11)
        {
            toggleFullScreen();
            return;
        }

        #if (!RELEASE_BUILD)
        if (event.key.keysym.scancode == SDL_SCANCODE_F1)
        {
            DebugOptions::debugOptionsMenuOpen = !DebugOptions::debugOptionsMenuOpen;
            return;
        }
        #endif
    }

    InputManager::processEvent(event);

    #if (!RELEASE_BUILD)
    ImGui_ImplSDL2_ProcessEvent(&event);
    #endif
}

void Game::toggleFullScreen()
{
    window.toggleFullscreen();

    // Set window stuff
    window.setIcon(icon);
    window.setVSync(true);

    // Reinitialise ImGui
    #if (!RELEASE_BUILD)
    ImGui_ImplSDL2_Shutdown();
    ImGui_ImplSDL2_InitForOpenGL(window.getSDLWindow(), window.getGLContext());
    #endif

    handleWindowResize(pl::Vector2<uint32_t>(window.getWidth(), window.getHeight()));
}

void Game::handleWindowResize(pl::Vector2<uint32_t> newSize)
{
    unsigned int newWidth = newSize.x;
    unsigned int newHeight = newSize.y;

    if (!window.getIsFullscreen())
    {
        newWidth = std::max(newSize.x, 1280U);
        newHeight = std::max(newSize.y, 720U);
    }

    window.setWindowSize(newWidth, newHeight);

    ResolutionHandler::setResolution({newWidth, newHeight});

    camera.instantUpdate(player.getPosition());

    // Resize chest item slots
    if (worldMenuState == WorldMenuState::Inventory && openedChestID != 0xFFFF)
    {
        InventoryGUI::chestOpened(getChestDataPool().getChestDataPtr(openedChestID));
    }

    // float afterScale = ResolutionHandler::getScale();

    // if (beforeScale != afterScale)
        // camera.handleScaleChange(beforeScale, afterScale, player.getPosition());
}

// -- Misc -- //

DayCycleManager& Game::getDayCycleManager(bool overrideMenuSwap)
{
    if (gameState == GameState::MainMenu && !overrideMenuSwap)
    {
        // Return a new day cycle manager while in main menu
        // Prevents world in background changing when a save is loaded
        static DayCycleManager tempDayCycleManager;
        return tempDayCycleManager;
    }
    
    return dayCycleManager;
}

ChunkManager& Game::getChunkManager(std::optional<PlanetType> planetTypeOverride)
{
    if (planetTypeOverride.has_value())
    {
        if (planetTypeOverride.value() >= 0)
        {
            return worldDatas.at(planetTypeOverride.value()).chunkManager;
        }
    }
    return worldDatas.at(locationState.getPlanetType()).chunkManager;
}

ProjectileManager& Game::getProjectileManager(std::optional<PlanetType> planetTypeOverride)
{
    if (planetTypeOverride.has_value())
    {
        if (planetTypeOverride.value() >= 0)
        {
            return worldDatas.at(planetTypeOverride.value()).projectileManager;
        }
    }
    return worldDatas.at(locationState.getPlanetType()).projectileManager;
}

BossManager& Game::getBossManager(std::optional<PlanetType> planetTypeOverride)
{
    if (planetTypeOverride.has_value())
    {
        if (planetTypeOverride.value() >= 0)
        {
            return worldDatas.at(planetTypeOverride.value()).bossManager;
        }
    }
    return worldDatas.at(locationState.getPlanetType()).bossManager;
}

LandmarkManager& Game::getLandmarkManager(std::optional<PlanetType> planetTypeOverride)
{
    if (planetTypeOverride.has_value())
    {
        if (planetTypeOverride.value() >= 0)
        {
            return worldDatas.at(planetTypeOverride.value()).landmarkManager;
        }
    }
    return worldDatas.at(locationState.getPlanetType()).landmarkManager;
}

RoomPool& Game::getStructureRoomPool(std::optional<PlanetType> planetTypeOverride)
{
    if (planetTypeOverride.has_value())
    {
        if (planetTypeOverride.value() >= 0)
        {
            return worldDatas.at(planetTypeOverride.value()).structureRoomPool;
        }
    }
    return worldDatas.at(locationState.getPlanetType()).structureRoomPool;
}

Room& Game::getRoomDestination(std::optional<RoomType> roomDestOverride)
{
    if (roomDestOverride.has_value())
    {
        if (roomDestOverride.value() >= 0)
        {
            return roomDestDatas.at(roomDestOverride.value()).roomDestination;
        }
    }

    assert(locationState.isInRoom());
    
    return roomDestDatas.at(locationState.getRoomDestType()).roomDestination;
}

ChestDataPool& Game::getChestDataPool(std::optional<LocationState> locationState)
{
    if (!locationState.has_value())
    {
        locationState = this->locationState;
    }

    if (locationState->isOnPlanet())
    {
        return worldDatas.at(locationState->getPlanetType()).chestDataPool;
    }
    else if (locationState->isInRoomDest())
    {
        return roomDestDatas.at(locationState->getRoomDestType()).chestDataPool;
    }

    // Default case
    return worldDatas.at(locationState->getPlanetType()).chestDataPool;
}

bool Game::isLocationStateInitialised(const LocationState& locationState)
{
    switch (locationState.getGameState())
    {
        case GameState::OnPlanet:
        {
            return worldDatas.contains(locationState.getPlanetType());
        }
        case GameState::InStructure:
        {
            return (worldDatas.contains(locationState.getPlanetType()) &&
                getStructureRoomPool(locationState.getPlanetType()).isIDValid(locationState.getInStructureID()));
        }
        case GameState::InRoomDestination:
        {
            return roomDestDatas.contains(locationState.getRoomDestType());
        }
    }

    return false;
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
    std::vector<uint8_t> noiseData(waterNoiseSize * waterNoiseSize * 4);
    std::vector<uint8_t> noiseTwoData(waterNoiseSize * waterNoiseSize * 4);

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
            noiseTwoData[index] = noiseValue * 255;
            noiseTwoData[index + 1] = noiseValue * 255;
            noiseTwoData[index + 2] = noiseValue * 255;
            noiseTwoData[index + 3] = 255;
        }
    }

    waterNoiseTextures[0].loadTexture(noiseData.data(), waterNoiseSize, waterNoiseSize);
    waterNoiseTextures[1].loadTexture(noiseTwoData.data(), waterNoiseSize, waterNoiseSize);

    // Pass noise textures into water shader
    pl::Shader* waterShader = Shaders::getShader(ShaderType::Water);
    waterShader->setUniformTexture("noise", waterNoiseTextures[0]);
    waterShader->setUniformTexture("noiseTwo", waterNoiseTextures[1]);
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
    pl::Rect<int> textureRect(80, 32, 8, 8);
    pl::Vector2f textureCentreRatio;

    bool shiftMode = InputManager::isActionActive(InputAction::UI_SHIFT);

    bool canQuickTransfer = false;
    if (!locationState.isNull())
    {
        canQuickTransfer = InventoryGUI::canQuickTransfer(mouseScreenPos, shiftMode, inventory, getChestDataPool().getChestDataPtr(openedChestID));
    }

    if (InputManager::isControllerActive())
    {
        if (gameState != GameState::MainMenu && (worldMenuState == WorldMenuState::Main ||
            worldMenuState == WorldMenuState::Inventory || worldMenuState == WorldMenuState::NPCShop))
        {
            if (canQuickTransfer)
            {
                textureRect = pl::Rect<int>(96, 48, 15, 15);
                textureCentreRatio = pl::Vector2f(1.0f / 3.0f, 1.0f / 3.0f);
            }
            else if (InputManager::isActionActive(InputAction::USE_TOOL))
            {
                textureRect = pl::Rect<int>(80, 64, 8, 8);
                textureCentreRatio = pl::Vector2f(0.5f, 0.5f);
            }
            else if (InputManager::isActionActive(InputAction::INTERACT))
            {
                textureRect = pl::Rect<int>(96, 64, 10, 10);
                textureCentreRatio = pl::Vector2f(0.5f, 0.5f);
            }
            else
            {
                textureRect = pl::Rect<int>(80, 48, 10, 10);
                textureCentreRatio = pl::Vector2f(0.5f, 0.5f);
            }
        }
        else
        {
            return;
        }
    }
    else
    {
        mouseScreenPos.x = std::max(std::min(mouseScreenPos.x, static_cast<float>(window.getWidth())), 0.0f);
        mouseScreenPos.y = std::max(std::min(mouseScreenPos.y, static_cast<float>(window.getHeight())), 0.0f);

        // Switch mouse cursor mode
        if (canQuickTransfer)
        {
            textureRect = pl::Rect<int>(96, 32, 12, 12);
        }
    }

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    pl::DrawData cursorDrawData;
    cursorDrawData.texture = TextureManager::getTexture(TextureType::UI);
    cursorDrawData.shader = Shaders::getShader(ShaderType::Default);
    cursorDrawData.position = mouseScreenPos;
    cursorDrawData.scale = pl::Vector2f(3, 3) * intScale;
    cursorDrawData.centerRatio = textureCentreRatio;
    cursorDrawData.textureRect = textureRect;
    
    TextureManager::drawSubTexture(window, cursorDrawData);
}

void Game::drawControllerGlyphs(const std::vector<std::pair<InputAction, std::string>>& actionStrings)
{
    static const std::unordered_map<ControllerGlyph, int> buttonGlyphXOffset = {
        {ControllerGlyph::BUTTON_A, 0 * 16},
        {ControllerGlyph::BUTTON_B, 1 * 16},
        {ControllerGlyph::BUTTON_X, 2 * 16},
        {ControllerGlyph::BUTTON_Y, 3 * 16},
        {ControllerGlyph::RIGHTSHOULDER, 4 * 16},
        {ControllerGlyph::LEFTSHOULDER, 5 * 16},
        {ControllerGlyph::RIGHTTRIGGER, 6 * 16},
        {ControllerGlyph::LEFTTRIGGER, 7 * 16},
        {ControllerGlyph::RIGHTSTICK, 8 * 16},
        {ControllerGlyph::LEFTSTICK, 9 * 16},
        {ControllerGlyph::SELECT, 10 * 16},
        {ControllerGlyph::START, 11 * 16}
    };

    pl::Vector2f resolution = static_cast<pl::Vector2f>(ResolutionHandler::getResolution());
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    static constexpr int GLYPH_SPACING = 50;
    static constexpr int GLYPH_X_PADDING = 70;

    for (int i = 0; i < actionStrings.size(); i++)
    {
        const auto& actionString = actionStrings[i];

        std::optional<ControllerGlyph> glyph = InputManager::getBoundActionControllerGlyph(actionString.first);

        // Set to no bind glyph by default
        pl::Rect<int> glyphTextureRect(192, 192, 16, 16);

        if (glyph.has_value())
        {
            glyphTextureRect.x = buttonGlyphXOffset.at(glyph.value());
            glyphTextureRect.y = 192 + InputManager::getGlyphType() * 16;
        }

        // Draw button glyph
        pl::DrawData glyphDrawData;
        glyphDrawData.texture = TextureManager::getTexture(TextureType::UI);
        glyphDrawData.shader = Shaders::getShader(ShaderType::Default);
        glyphDrawData.position = pl::Vector2f(resolution.x - GLYPH_X_PADDING / 2.0f * intScale, resolution.y - (i + 1) * GLYPH_SPACING * intScale);
        glyphDrawData.centerRatio = pl::Vector2f(0.5f, 0.5f);
        glyphDrawData.scale = pl::Vector2f(3, 3) * intScale;
        glyphDrawData.textureRect = glyphTextureRect;

        spriteBatch.draw(window, glyphDrawData);

        // Draw action text
        pl::TextDrawData textDrawData;
        textDrawData.text = actionString.second;
        textDrawData.position = pl::Vector2f(resolution.x, resolution.y - (i + 1) * GLYPH_SPACING * intScale);
        textDrawData.size = 24 * intScale;
        textDrawData.color = pl::Color(255, 255, 255);
        textDrawData.outlineColor = pl::Color(46, 34, 47);
        textDrawData.outlineThickness = 2 * intScale;
        textDrawData.containOnScreenX = true;
        textDrawData.containPaddingRight = GLYPH_X_PADDING * intScale;
        textDrawData.centeredY = true;

        TextDraw::drawText(window, textDrawData);
    }
}

#if (!RELEASE_BUILD)
void Game::drawDebugMenu(float dt)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (!DebugOptions::debugOptionsMenuOpen)
    {
        ImGui::EndFrame();
        return;
    }

    ImGui::Begin("Debug Options", &DebugOptions::debugOptionsMenuOpen);

    // Debug info
    std::vector<std::string> debugStrings = {
        GAME_VERSION,
        std::to_string(static_cast<int>(1.0f / dt)) + "FPS",
        ((gameState == GameState::OnPlanet || gameState == GameState::InStructure) ?
            std::to_string(getChunkManager().getLoadedChunkCount()) + " Chunks loaded" : ""),
        ((gameState == GameState::OnPlanet || gameState == GameState::InStructure) ?
            std::to_string(getChunkManager().getGeneratedChunkCount()) + " Chunks generated" : ""),
        std::to_string(worldDatas.size()) + " world datas active",
        std::to_string(roomDestDatas.size()) + " roomdest datas active",
        "Seed: " + std::to_string(planetSeed),
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

    if (!networkHandler.isClient())
    {
        if (ImGui::Button("Save"))
        {
            saveGame();
        }
    }

    ImGui::Spacing();

    ImGui::Checkbox("Smooth Lighting", &smoothLighting);

    float time = dayCycleManager.getCurrentTime();
    if (!networkHandler.isClient())
    {
        if (ImGui::SliderFloat("Day time", &time, 0.0f, dayCycleManager.getDayLength()))
        {
            dayCycleManager.setCurrentTime(time);
        }
    }
    else
    {
        ImGui::Text(("Day time: " + std::to_string(time)).c_str());
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

    ImGui::Checkbox("Crazy Attack", &DebugOptions::crazyAttack);

    ImGui::Text(("Weather value: " + std::to_string(weatherSystem.sampleWeatherFunction(gameTime))).c_str());
    ImGui::Text(("Weather transition: " + std::to_string(weatherSystem.getDestinationTransitionProgress())).c_str());

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
#endif