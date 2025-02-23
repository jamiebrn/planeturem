#include "Game.hpp"

// FIX: Lighting sometimes momentarily breaks when crossing world boundary

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

    // Disable joystick events (using SDL for input)
    window.setJoystickSensorEventsEnabled(false);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
    {
        std::cerr << "Failed to initialise SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    sdlWindow = SDL_CreateWindowFrom(reinterpret_cast<void*>(window.getSystemHandle()));
    if (!sdlWindow)
    {
        std::cerr << "Failed to create SDL window from handle: " << SDL_GetError() << std::endl;
        window.close();
        SDL_Quit();
        return false;
    }

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
    {
        SteamUserStats()->RequestCurrentStats();
        SteamNetworkingUtils()->InitRelayNetworkAccess();
    }

    // Randomise
    srand(time(NULL));

    loadOptions();

    InputManager::initialise(sdlWindow);

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

    mainMenuGUI.initialise();

    //menuScreenshotIndex = 0;
    //menuScreenshotTimer = 0.0f;

    openedChestID = 0xFFFF;

    musicGapTimer = 0.0f;
    musicGap = 0.0f;

    player = Player(sf::Vector2f(0, 0));
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
        mouseScreenPos = InputManager::getMousePosition(sdlWindow, dt);

        window.setView(view);

        handleSDLEvents();

        if (multiplayerGame)
        {
            receiveMessages();
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

        drawMouseCursor();

        #if (!RELEASE_BUILD)
        drawDebugMenu(dt);
        #endif

        ImGui::SetMouseCursor(ImGui::GetIO().WantCaptureMouse ? ImGuiMouseCursor_Arrow : ImGuiMouseCursor_None);

        ImGui::SFML::Render(window);

        window.display();
    }

    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();
}

#if (!RELEASE_BUILD)
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
#endif

// -- Main Menu -- //

void Game::runMainMenu(float dt)
{
    for (auto event = sf::Event{}; window.pollEvent(event);)
    {
        handleEventsWindow(event);

        mainMenuGUI.handleEvent(event);
    }

    // sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    mainMenuGUI.update(dt, mouseScreenPos, *this, projectileManager);

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
    // sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    bool shiftMode = InputManager::isActionActive(InputAction::UI_SHIFT);

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
                            InventoryGUI::handleLeftClick(mouseScreenPos, shiftMode, inventory, armourInventory, chestDataPool.getChestDataPtr(openedChestID));
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
            InputManager::recentreControllerCursor(sdlWindow);
        }
    }

    //
    // -- NETWORKING --
    //
    if (multiplayerGame)
    {
        if (isLobbyHost)
        {
            sendHostMessages();
        }
        else
        {
            sendClientMessages();
        }
    }

    //
    // -- UPDATING --
    //

    if (worldMenuState != WorldMenuState::PauseMenu || (multiplayerGame && networkPlayers.size() > 0))
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
            {
                InventoryGUI::drawHotbar(window, mouseScreenPos, inventory);
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
                if (InventoryGUI::handleControllerInput(inventory, armourInventory, chestDataPool.getChestDataPtr(openedChestID)))
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
                    chestDataPtr = chestDataPool.getChestDataPtr(openedChestID);
                }

                InventoryGUI::draw(window, gameTime, mouseScreenPos, inventory, armourInventory, chestDataPtr);

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
        }
    }
    else
    {
        HealthGUI::drawDeadPrompt(window);
    }

    if (worldMenuState == WorldMenuState::PauseMenu)
    {
        std::optional<uint64_t> lobbyId;
        if (multiplayerGame)
        {
            lobbyId = steamLobbyId;
        }
        std::optional<PauseMenuEventType> pauseMenuEvent = mainMenuGUI.createAndDrawPauseMenu(window, dt, gameTime, steamInitialised, lobbyId);

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
                    createLobby();
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
    // sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    int worldSize = chunkManager.getWorldSize();

    // Update cursor
    Cursor::updateTileCursor(camera.screenToWorldTransform(mouseScreenPos), dt, chunkManager, player.getCollisionRect(), InventoryGUI::getHeldItemType(inventory),
        player.getTool());

    // Update player
    bool wrappedAroundWorld = false;
    sf::Vector2f wrapPositionDelta;

    if (!isStateTransitioning())
    {
        player.update(dt, camera.screenToWorldTransform(mouseScreenPos), chunkManager, enemyProjectileManager, wrappedAroundWorld, wrapPositionDelta);
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

        // Wrap weather particles
        weatherSystem.handleWorldWrap(wrapPositionDelta);
    }

    // Update (loaded) chunks
    // Enable / disable chunk generation depending on multiplayer state
    bool isClient = (multiplayerGame && !isLobbyHost);
    std::vector<ChunkPosition> chunksToRequestFromHost;

    bool modifiedChunks = chunkManager.updateChunks(*this, camera, isClient, &chunksToRequestFromHost);

    if (multiplayerGame && !isLobbyHost && chunksToRequestFromHost.size() > 0)
    {
        requestChunksFromHost(chunksToRequestFromHost);
    }

    chunkManager.updateChunksObjects(*this, dt);
    chunkManager.updateChunksEntities(dt, projectileManager, *this);

    // If modified chunks, force a lighting recalculation
    if (modifiedChunks)
    {
        lightingTick = LIGHTING_TICK;
    }
    
    // Get nearby crafting stations
    nearbyCraftingStationLevels = chunkManager.getNearbyCraftingStationLevels(player.getChunkInside(worldSize), player.getChunkTileInside(worldSize), 4);

    // Update bosses
    bossManager.update(*this, projectileManager, enemyProjectileManager, chunkManager, player, dt, gameTime);

    // Update projectiles
    projectileManager.update(dt);
    enemyProjectileManager.update(dt);

    weatherSystem.update(dt, gameTime, camera, chunkManager);

    // Test item pickups colliding
    std::optional<ItemPickupReference> itemPickupColliding = chunkManager.getCollidingItemPickup(player.getCollisionRect(), gameTime);
    if (itemPickupColliding.has_value())
    {
        const ItemPickup* itemPickupPtr = chunkManager.getChunk(itemPickupColliding->chunk)->getItemPickup(itemPickupColliding->id);

        if (itemPickupPtr != nullptr)
        {
            // Only actually add item to inventory if solo or is host
            // Host will give client item
            bool modifyInventory = (!multiplayerGame || isLobbyHost);

            int amountAdded = inventory.addItem(itemPickupPtr->getItemType(), 1, modifyInventory, false, modifyInventory);

            if (amountAdded > 0)
            {
                // Only delete pickup if playing solo or is host
                // if (!multiplayerGame || isLobbyHost)
                // {
                chunkManager.deleteItemPickup(itemPickupColliding.value());
                // }
    
                // Play pickup sound
                const std::vector<SoundType> pickupSounds = {SoundType::Pop0, SoundType::Pop1, SoundType::Pop2, SoundType::Pop3};
                Sounds::playSound(pickupSounds[Helper::randInt(0, pickupSounds.size() - 1)], 30.0f);

                // Networking
                if (multiplayerGame && (!isLobbyHost || networkPlayers.size() > 0))
                {
                    PacketDataItemPickupDeleted packetData;
                    packetData.pickupDeleted = itemPickupColliding.value();
                    Packet packet;
                    packet.set(packetData);

                    if (isLobbyHost)
                    {
                        sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
                    }
                    else
                    {
                        sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
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

void Game::drawOnPlanet(float dt)
{
    // Get world objects
    std::vector<WorldObject*> worldObjects = chunkManager.getChunkObjects();
    std::vector<WorldObject*> entities = chunkManager.getChunkEntities();
    std::vector<WorldObject*> itemPickups = chunkManager.getItemPickups();
    std::vector<WorldObject*> weatherParticles = weatherSystem.getWeatherParticles();
    worldObjects.insert(worldObjects.end(), entities.begin(), entities.end());
    worldObjects.insert(worldObjects.end(), itemPickups.begin(), itemPickups.end());
    worldObjects.insert(worldObjects.end(), weatherParticles.begin(), weatherParticles.end());
    worldObjects.push_back(&player);
    bossManager.getBossWorldObjects(worldObjects);

    // Add network players
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        worldObjects.push_back(&iter->second);
    }

    drawWorld(worldTexture, dt, worldObjects, chunkManager, camera);
    drawLighting(dt, worldObjects);

    // UI
    // sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

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

    unsigned char ambientRedLight = Helper::lerp(2, 255 * weatherSystem.getRedLightBias(), lightLevel);
    unsigned char ambientGreenLight = Helper::lerp(7, 244 * weatherSystem.getGreenLightBias(), lightLevel);
    unsigned char ambientBlueLight = Helper::lerp(14, 234 * weatherSystem.getBlueLightBias(), lightLevel);

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
    // sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

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
        chunkManager.updateChunksObjects(*this, dt);
        chunkManager.updateChunksEntities(dt, projectileManager, *this);
            
        weatherSystem.update(dt, gameTime, camera, chunkManager);

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
    sf::Vector2f mouseWorldPos = camera.screenToWorldTransform(mouseScreenPos);
    
    // Swing pickaxe
    player.useTool(projectileManager, inventory, mouseWorldPos, *this);

    if (gameState != GameState::OnPlanet)
        return;

    if (!player.canReachPosition(mouseWorldPos))
        return;

    // Get current tool damage amount
    ToolType currentTool = player.getTool();

    const ToolData& toolData = ToolDataLoader::getToolData(currentTool);

    hitObject(Cursor::getSelectedChunk(chunkManager.getWorldSize()), Cursor::getSelectedChunkTile(), toolData.damage);
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

    sf::Vector2f mouseWorldPos = camera.screenToWorldTransform(mouseScreenPos);

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
    player.useTool(projectileManager, inventory, mouseWorldPos, *this);

    player.swingFishingRod(mouseWorldPos, Cursor::getSelectedWorldTile(chunkManager.getWorldSize()));
}

void Game::attemptUseToolWeapon()
{
    if (gameState != GameState::OnPlanet)
        return;

    sf::Vector2f mouseWorldPos = camera.screenToWorldTransform(mouseScreenPos);

    player.useTool(projectileManager, inventory, mouseWorldPos, *this);
}

void Game::hitObject(ChunkPosition chunk, sf::Vector2i tile, int damage, bool sentFromHost, std::optional<uint64_t> userId)
{
    // If multiplayer and this client attempted to hit object, send hit object packet to host
    if (multiplayerGame && !isLobbyHost && !sentFromHost)
    {
        PacketDataObjectHit packetData;
        packetData.objectHit.chunk = chunk;
        packetData.objectHit.tile = tile;
        packetData.damage = damage;
        packetData.userId = SteamUser()->GetSteamID().ConvertToUint64();

        Packet packet;
        packet.set(packetData);
        sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
        return;
    }

    // Not multiplayer game / is host / hit object packet sent from host

    bool canDestroyObject = chunkManager.canDestroyObject(chunk, tile, player.getCollisionRect());

    if (!canDestroyObject)
        return;

    BuildableObject* selectedObject = chunkManager.getChunkObject(chunk, tile);

    if (selectedObject)
    {
        // Only drop items if playing solo or is host of lobby
        // In multiplayer, host handles creation of all pickups and alerts clients of new pickups
        bool dropItems = (!multiplayerGame || isLobbyHost);

        bool destroyed = selectedObject->damage(damage, *this, chunkManager, particleSystem, dropItems);

        // Alert network players if host
        if (multiplayerGame && isLobbyHost)
        {
            PacketDataObjectHit packetData;
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
            sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
        }

        if (destroyed)
        {
            // Only apply screenshake if this client destroyed object
            bool applyScreenShake = true;
            
            if (multiplayerGame)
            {
                // If host, alert clients of object destruction
                if (isLobbyHost)
                {
                    PacketDataObjectDestroyed objectDestroyedPacketData;
                    objectDestroyedPacketData.objectReference.chunk = chunk;
                    objectDestroyedPacketData.objectReference.tile = tile;
                    objectDestroyedPacketData.userId = userId.has_value() ? userId.value() : SteamUser()->GetSteamID().ConvertToUint64(); 
                    Packet packet;
                    packet.set(objectDestroyedPacketData);
                    sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
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

void Game::buildObject(ChunkPosition chunk, sf::Vector2i tile, ObjectType objectType, bool sentFromHost)
{
    // If multiplayer game and this client builds object, send build object packet to host
    if (multiplayerGame && !isLobbyHost && !sentFromHost)
    {
        PacketDataObjectBuilt packetData;
        packetData.objectReference.chunk = chunk;
        packetData.objectReference.tile = tile;
        packetData.objectType = objectType;
        packetData.userId = SteamUser()->GetSteamID().ConvertToUint64();

        Packet packet;
        packet.set(packetData);
        sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
        return;
    }

    // Not multiplayer game / sent from host / is host

    if (multiplayerGame && isLobbyHost)
    {
        // Send build object packets to clients
        PacketDataObjectBuilt packetData;
        packetData.objectReference.chunk = chunk;
        packetData.objectReference.tile = tile;
        packetData.objectType = objectType;
        packetData.userId = SteamUser()->GetSteamID().ConvertToUint64();

        Packet packet;
        packet.set(packetData);
        sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
    }

    // If sent from host and client does not have chunk, request it
    if (multiplayerGame && !isLobbyHost && sentFromHost)
    {
        Chunk* chunkPtr = chunkManager.getChunk(chunk);
        if (!chunkPtr)
        {
            std::vector<ChunkPosition> requestedChunks = {chunk};
            requestChunksFromHost(requestedChunks);
            return;
        }
    }

    // Build object
    chunkManager.setObject(chunk, tile, objectType, *this);
    
    // Create build particles
    BuildableObject* placedObject = chunkManager.getChunkObject(chunk, tile);
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

void Game::testMeleeCollision(const std::vector<HitRect>& hitRects)
{
    chunkManager.testChunkEntityHitCollision(hitRects, *this, gameTime);
    bossManager.testHitRectCollision(hitRects);
}

void Game::itemPickupsCreated(const std::vector<ItemPickupReference>& itemPickupsCreated)
{
    if (!multiplayerGame || !isLobbyHost)
    {
        return;
    }

    // Alert clients of item pickups created
    PacketDataItemPickupsCreated packetData;
    for (auto& itemPickupReference : itemPickupsCreated)
    {
        Chunk* chunkPtr = chunkManager.getChunk(itemPickupReference.chunk);
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
    sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
}

void Game::catchRandomFish(sf::Vector2i fishedTile)
{
    const BiomeGenData* biomeGenData = chunkManager.getChunkBiome(ChunkPosition(fishedTile.x / CHUNK_TILE_SIZE, fishedTile.y / CHUNK_TILE_SIZE));

    // Check for nullptr
    if (!biomeGenData)
        return;

    std::vector<ItemPickupReference> itemPickupsCreatedVector;
    
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
                sf::Vector2f spawnPos = player.getPosition() + sf::Vector2f(
                    Helper::randFloat(-TILE_SIZE_PIXELS_UNSCALED / 2.0f, TILE_SIZE_PIXELS_UNSCALED / 2.0f),
                    Helper::randFloat(-TILE_SIZE_PIXELS_UNSCALED / 2.0f, TILE_SIZE_PIXELS_UNSCALED / 2.0f)
                );

                itemPickupsCreatedVector.push_back(chunkManager.addItemPickup(ItemPickup(spawnPos, fishCatchData.itemCatch, gameTime)).value());
            }

            break;
        }
    }

    itemPickupsCreated(itemPickupsCreatedVector);
}

void Game::attemptObjectInteract()
{
    if (worldMenuState != WorldMenuState::Main && worldMenuState != WorldMenuState::Inventory)
    {
        return;
    }

    // Get mouse position in screen space and world space
    sf::Vector2f mouseWorldPos = camera.screenToWorldTransform(mouseScreenPos);

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

    if (objectType < 0)
        return;

    if (!player.canReachPosition(camera.screenToWorldTransform(mouseScreenPos)))
    {
        return;
    }

    if (!chunkManager.canPlaceObject(Cursor::getSelectedChunk(chunkManager.getWorldSize()), Cursor::getSelectedChunkTile(), objectType, player.getCollisionRect()))
    {
        return;
    }
    
    InventoryGUI::subtractHeldItem(inventory);

    buildObject(Cursor::getSelectedChunk(chunkManager.getWorldSize()), Cursor::getSelectedChunkTile(), objectType);
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
    
    if (!player.canReachPosition(camera.screenToWorldTransform(mouseScreenPos)))
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

    bool inRange = player.canReachPosition(camera.screenToWorldTransform(mouseScreenPos));

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
    if (chunkManager.canPlaceLand(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile()) && player.canReachPosition(camera.screenToWorldTransform(mouseScreenPos)))
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
    sf::Vector2f mouseWorldPos = camera.screenToWorldTransform(mouseScreenPos);

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

    if (gameState == GameState::OnPlanet && (!multiplayerGame || isLobbyHost))
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

    if (!multiplayerGame || isLobbyHost)
    {
        chunkManager.updateChunks(*this, camera);
    }

    weatherSystem = WeatherSystem(gameTime, chunkManager.getSeed() + chunkManager.getPlanetType());
    weatherSystem.presimulateWeather(gameTime, camera, chunkManager);

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
        case GameState::MainMenu:
        {
            ResolutionHandler::overrideZoom(0);
            break;
        }
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

void Game::startNewGame(int seed, std::optional<std::string> overridePlanetName)
{
    // setWorldSeedFromInput();
    networkPlayers.clear();
    multiplayerGame = false;
    lobbyHost = false;

    player = Player(sf::Vector2f(0, 0));
    inventory = InventoryData(32);
    armourInventory = InventoryData(3);
    giveStartingInventory();
    InventoryGUI::reset();

    chunkManager.setSeed(seed);

    initialiseNewPlanet(PlanetGenDataLoader::getPlanetTypeFromName(overridePlanetName.has_value() ? overridePlanetName.value() : "Earthlike"));

    dayCycleManager.setCurrentTime(dayCycleManager.getDayLength() * 0.5f);
    dayCycleManager.setCurrentDay(1);

    saveSessionPlayTime = 0.0f;

    bossManager.clearBosses();
    projectileManager.clear();
    enemyProjectileManager.clear();
    landmarkManager.clear();

    gameTime = 0.0f;

    weatherSystem = WeatherSystem(gameTime, seed + chunkManager.getPlanetType());
    weatherSystem.presimulateWeather(gameTime, camera, chunkManager);

    camera.instantUpdate(player.getPosition());

    // Only generate chunks if solo / lobby host
    if (!multiplayerGame || isLobbyHost)
    {
        chunkManager.updateChunks(*this, camera);
    }
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
    playerGameSave.maxHealth = player.getMaxHealth();
    playerGameSave.time = dayCycleManager.getCurrentTime();
    playerGameSave.day = dayCycleManager.getCurrentDay();

    for (ItemType itemType : InventoryGUI::getSeenRecipes())
    {
        playerGameSave.recipesSeen.insert(ItemDataLoader::getItemData(itemType).name);
    }

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

    networkPlayers.clear();
    multiplayerGame = false;
    lobbyHost = false;

    player = Player(sf::Vector2f(0, 0), playerGameSave.maxHealth);

    InventoryGUI::reset();

    std::unordered_set<ItemType> recipesSeen;
    for (const std::string& itemName : playerGameSave.recipesSeen)
    {
        recipesSeen.insert(ItemDataLoader::getItemTypeFromName(itemName));
    }
    InventoryGUI::setSeenRecipes(recipesSeen);

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
        
    // Sync time-based systems, e.g. weather
    gameTime = playerGameSave.timePlayed;

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

            // Simulate weather outside of room
            camera.instantUpdate(structureEnteredPos);
        }
        else
        {
            player.setPosition(planetGameSave.playerLastPlanetPos);
            camera.instantUpdate(player.getPosition());
        }
        
        weatherSystem = WeatherSystem(gameTime, chunkManager.getSeed() + chunkManager.getPlanetType());
        weatherSystem.presimulateWeather(gameTime, camera, chunkManager);
        
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
        
        // Simulate weather outside of room
        camera.instantUpdate(structureEnteredPos);
    }
    else
    {
        overrideState(GameState::OnPlanet);
        player.setPosition(planetGameSave.playerLastPlanetPos);
        
        camera.instantUpdate(player.getPosition());
    }
    
    weatherSystem = WeatherSystem(gameTime, chunkManager.getSeed() + chunkManager.getPlanetType());
    weatherSystem.presimulateWeather(gameTime, camera, chunkManager);

    camera.instantUpdate(player.getPosition());

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
    optionsSave.screenShakeEnabled = Camera::getScreenShakeEnabled();
    optionsSave.controllerGlyphType = InputManager::getGlyphType();

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
}

void Game::quitWorld()
{
    if (!multiplayerGame || isLobbyHost)
    {
        saveGame();
    }

    if (multiplayerGame)
    {
        leaveLobby();
    }

    currentSaveFileSummary.name = "";
    startChangeStateTransition(GameState::MainMenu);
    mainMenuGUI.initialise();
    Sounds::stopMusic();
}


// -- Multiplayer --

void Game::createLobby()
{
    networkPlayers.clear();
    SteamAPICall_t steamAPICall = SteamMatchmaking()->CreateLobby(ELobbyType::k_ELobbyTypeFriendsOnly, 8);
    m_SteamCallResultCreateLobby.Set(steamAPICall, this, &Game::callbackLobbyCreated);
}

void Game::callbackLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure)
{
    if (pCallback->m_ulSteamIDLobby == 0)
    {
        std::cout << "Lobby creation failed\n";
        return;
    }

    std::cout << "Created lobby " << pCallback->m_ulSteamIDLobby << "\n";
    isLobbyHost = true;
    lobbyHost = SteamUser()->GetSteamID().ConvertToUint64();
    multiplayerGame = true;
}

void Game::leaveLobby()
{
    if (isLobbyHost)
    {
        SteamMatchmaking()->SetLobbyJoinable(steamLobbyId, false);

        // Alert clients of host leaving
        Packet packet;
        packet.type = PacketType::HostQuit;

        for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
        {
            SteamNetworkingIdentity identity;
            identity.SetSteamID64(iter->first);
            packet.sendToUser(identity, k_nSteamNetworkingSend_Reliable, 0);
            SteamNetworkingMessages()->CloseSessionWithUser(identity);
        }
    }
    else
    {
        SteamNetworkingIdentity hostIdentity;
        hostIdentity.SetSteamID64(lobbyHost);
        SteamNetworkingMessages()->CloseSessionWithUser(hostIdentity);
    }
    
    SteamMatchmaking()->LeaveLobby(steamLobbyId);
    isLobbyHost = false;
    multiplayerGame = false;
}

void Game::joinWorld(const PacketDataJoinInfo& joinInfo)
{
    startNewGame(joinInfo.seed, joinInfo.planetName);

    dayCycleManager.setCurrentTime(joinInfo.time);
    dayCycleManager.setCurrentDay(joinInfo.day);

    gameTime = joinInfo.gameTime;

    weatherSystem = WeatherSystem(gameTime, joinInfo.seed + chunkManager.getPlanetType());
    weatherSystem.presimulateWeather(gameTime, camera, chunkManager);

    chunkRequestsOutstanding.clear();
}

void Game::callbackLobbyJoinRequested(GameLobbyJoinRequested_t* pCallback)
{
    SteamMatchmaking()->JoinLobby(pCallback->m_steamIDLobby);
    multiplayerGame = true;
}

void Game::callbackLobbyEnter(LobbyEnter_t* pCallback)
{
    steamLobbyId = pCallback->m_ulSteamIDLobby;
    std::cout << "Joined lobby " << steamLobbyId << "\n";
    multiplayerGame = true;
}

void Game::callbackLobbyUpdated(LobbyChatUpdate_t* pCallback)
{
    SteamNetworkingIdentity userIdentity;
    userIdentity.SetSteamID64(pCallback->m_ulSteamIDUserChanged);
    
    if (isLobbyHost)
    {
        Packet packet;
        
        if (pCallback->m_rgfChatMemberStateChange & k_EChatMemberStateChangeEntered)
        {
            packet.type = PacketType::JoinQuery;
            EResult result = packet.sendToUser(userIdentity, k_nSteamNetworkingSend_Reliable, 0);
            if (result == EResult::k_EResultOK)
            {
                std::cout << "Sent join query successfully\n";
            }
            else if (result == EResult::k_EResultNoConnection)
            {
                std::cout << "Could not send join query\n";
            }
        }
        else
        {
            deleteNetworkPlayer(pCallback->m_ulSteamIDUserChanged);
        }
    }
    else
    {
        // Test for host disconnect
        if (multiplayerGame)
        {
            // Test while in game
            if (pCallback->m_ulSteamIDUserChanged == lobbyHost)
            {
                quitWorld();
            }
        }
        else
        {
            // Test while in lobby, but not in game (rare case)
            if (pCallback->m_ulSteamIDUserChanged == SteamMatchmaking()->GetLobbyOwner(steamLobbyId).ConvertToUint64())
            {
                leaveLobby();
            }
        }
    }
}

void Game::callbackMessageSessionRequest(SteamNetworkingMessagesSessionRequest_t* pCallback)
{
    SteamNetworkingMessages()->AcceptSessionWithUser(pCallback->m_identityRemote);
}

void Game::registerNetworkPlayer(uint64_t id, bool notify)
{
    // Alert connected players
    if (isLobbyHost)
    {
        Packet packet;
        packet.type = PacketType::PlayerJoined;
        packet.data.resize(sizeof(id));
        memcpy(packet.data.data(), &id, sizeof(id));
    
        for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
        {
            SteamNetworkingIdentity identity;
            identity.SetSteamID64(iter->first);
            packet.sendToUser(identity, k_nSteamNetworkingSend_Reliable, 0);
        }
    }

    if (notify)
    {
        InventoryGUI::pushItemPopup(ItemCount(0, 1), false, std::string(SteamFriends()->GetFriendPersonaName(id)) + " joined");
    }

    networkPlayers[id] = Player(sf::Vector2f(0, 0));
}

void Game::deleteNetworkPlayer(uint64_t id)
{
    if (!networkPlayers.contains(id))
    {
        return;
    }
    
    networkPlayers.erase(id);

    // Alert connected players
    if (isLobbyHost)
    {
        Packet packet;
        packet.type = PacketType::PlayerDisconnected;
        packet.data.resize(sizeof(id));
        memcpy(packet.data.data(), &id, sizeof(id));
    
        for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
        {
            SteamNetworkingIdentity identity;
            identity.SetSteamID64(iter->first);
            packet.sendToUser(identity, k_nSteamNetworkingSend_Reliable, 0);
        }
    }

    InventoryGUI::pushItemPopup(ItemCount(0, 1), false, std::string(SteamFriends()->GetFriendPersonaName(id)) + " disconnected");
}

void Game::receiveMessages()
{
    static const int MAX_MESSAGES = 10;

    SteamNetworkingMessage_t* messages[MAX_MESSAGES];
    int messageCount = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, messages, MAX_MESSAGES);

    for (int i = 0; i < messageCount; i++)
    {
        Packet packet;
        packet.deserialise((char*)messages[i]->GetData(), messages[i]->GetSize());

        // Process packet
        if (packet.type == PacketType::JoinReply && isLobbyHost)
        {
            const char* steamName = SteamFriends()->GetFriendPersonaName(messages[i]->m_identityPeer.GetSteamID());
            std::cout << "Player joined: " << steamName << " (" << messages[i]->m_identityPeer.GetSteamID64() << ")\n";
            
            // Send world info
            PacketDataJoinInfo packetData;
            packetData.seed = chunkManager.getSeed();
            packetData.gameTime = gameTime;
            packetData.time = dayCycleManager.getCurrentTime();
            packetData.day = dayCycleManager.getCurrentDay();
            packetData.planetName = PlanetGenDataLoader::getPlanetGenData(chunkManager.getPlanetType()).name;
            
            packetData.currentPlayers.push_back(SteamUser()->GetSteamID().ConvertToUint64());
            for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
            {
                packetData.currentPlayers.push_back(iter->first);
            }

            registerNetworkPlayer(messages[i]->m_identityPeer.GetSteamID64());
            
            Packet packetToSend;
            packetToSend.set(packetData);
            packetToSend.sendToUser(messages[i]->m_identityPeer, k_nSteamNetworkingSend_Reliable, 0);
        }
        else if (packet.type == PacketType::JoinQuery)
        {
            Packet packetToSend;
            packetToSend.type = PacketType::JoinReply;
            
            std::cout << "Sending join reply to user " << messages[i]->m_identityPeer.GetSteamID64() << "\n";
            
            packetToSend.sendToUser(messages[i]->m_identityPeer, k_nSteamNetworkingSend_Reliable, 0);
        }
        else if (packet.type == PacketType::JoinInfo)
        {
            // Deserialise packet data
            PacketDataJoinInfo packetData;
            packetData.deserialise(packet.data);

            // Load into world
            joinWorld(packetData);

            // Set lobby host
            lobbyHost = messages[i]->m_identityPeer.GetSteamID64();
            isLobbyHost = false;

            multiplayerGame = true;
            
            for (uint64_t player : packetData.currentPlayers)
            {
                registerNetworkPlayer(player, false);
                std::cout << "Registered existing player " << SteamFriends()->GetFriendPersonaName(CSteamID(player)) << "\n";
            }
        }
        else if (packet.type == PacketType::PlayerJoined && !isLobbyHost)
        {
            uint64_t id;
            memcpy(&id, packet.data.data(), sizeof(id));
            registerNetworkPlayer(id);
        }
        else if (packet.type == PacketType::PlayerDisconnected && !isLobbyHost)
        {    
            uint64_t id;
            memcpy(&id, packet.data.data(), sizeof(id));
            deleteNetworkPlayer(id);
        }
        else if (packet.type == PacketType::HostQuit && !isLobbyHost)
        {
            quitWorld();
        }
        else if (packet.type == PacketType::WorldInfo && !isLobbyHost)
        {
            PacketDataWorldInfo worldInfo;
            worldInfo.deserialise(packet.data);
            worldInfo.applyPingEstimate();

            gameTime = worldInfo.gameTime;
            dayCycleManager.setCurrentDay(worldInfo.day);
            dayCycleManager.setCurrentTime(worldInfo.time);
        }
        else if (packet.type == PacketType::PlayerInfo)
        {
            if (isLobbyHost)
            {
                if (!networkPlayers.contains(messages[i]->m_identityPeer.GetSteamID64()))
                {
                    registerNetworkPlayer(messages[i]->m_identityPeer.GetSteamID64());
                }
            }

            PacketDataPlayerInfo packetData;
            packetData.deserialise(packet.data);

            if (networkPlayers.contains(packetData.steamID))
            {
                std::string playerName = SteamFriends()->GetFriendPersonaName(CSteamID(packetData.steamID));

                // Translate player position to wrap around world, relative to player
                sf::Vector2f playerPos = chunkManager.translatePositionAroundWorld(sf::Vector2f(packetData.positionX, packetData.positionY), player.getPosition());
                packetData.positionX = playerPos.x;
                packetData.positionY = playerPos.y;

                networkPlayers[packetData.steamID].setNetworkPlayerInfo(packetData, playerName);
            }
        }
        else if (packet.type == PacketType::ObjectHit)
        {
            PacketDataObjectHit packetData;
            packetData.deserialise(packet.data);
            bool sentFromHost = !isLobbyHost;
            hitObject(packetData.objectHit.chunk, packetData.objectHit.tile, packetData.damage, sentFromHost, packetData.userId);
        }
        else if (packet.type == PacketType::ObjectBuilt)
        {
            PacketDataObjectBuilt packetData;
            packetData.deserialise(packet.data);
            bool sentFromHost = !isLobbyHost;
            buildObject(packetData.objectReference.chunk, packetData.objectReference.tile, packetData.objectType, sentFromHost);
        }
        else if (packet.type == PacketType::ObjectDestroyed && !isLobbyHost)
        {
            PacketDataObjectDestroyed packetData;
            packetData.deserialise(packet.data);
            chunkManager.deleteObject(packetData.objectReference.chunk, packetData.objectReference.tile);
        }
        else if (packet.type == PacketType::ItemPickupsCreated)
        {
            PacketDataItemPickupsCreated packetData;
            packetData.deserialise(packet.data);
            
            // Create item pickups sent from host
            for (auto& itemPickupPair : packetData.createdPickups)
            {
                Chunk* chunkPtr = chunkManager.getChunk(itemPickupPair.first.chunk);
                if (!chunkPtr)
                {
                    std::cout << "ERROR: Failed to create item pickup sent from host in null chunk (" << itemPickupPair.first.chunk.x <<
                        ", " << itemPickupPair.first.chunk.y << ")\n";
                    continue;
                }

                // Denormalise pickup position from chunk-relative to world position
                itemPickupPair.second.setPosition(itemPickupPair.second.getPosition() + chunkPtr->getWorldPosition());

                chunkPtr->addItemPickup(itemPickupPair.second, itemPickupPair.first.id);
            }

            // If host, redistribute pickups created message to clients
            if (isLobbyHost)
            {
                sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
            }
        }
        else if (packet.type == PacketType::ItemPickupDeleted)
        {
            PacketDataItemPickupDeleted packetData;
            packetData.deserialise(packet.data);
            
            // If host, redistribute to clients
            if (isLobbyHost)
            {
                sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);

                Chunk* chunkPtr = chunkManager.getChunk(packetData.pickupDeleted.chunk);
                if (chunkPtr)
                {
                    ItemPickup* itemPickupPtr = chunkPtr->getItemPickup(packetData.pickupDeleted.id);
                    if (itemPickupPtr)
                    {
                        // Give item to client
                        PacketDataInventoryAddItem itemPacketData;
                        itemPacketData.itemType = itemPickupPtr->getItemType();
                        itemPacketData.amount = 1;
                        Packet itemPacket;
                        itemPacket.set(itemPacketData);

                        itemPacket.sendToUser(messages[i]->m_identityPeer, k_nSteamNetworkingSend_Reliable, 0);
                    }
                }
            }

            // Delete pickup from chunk manager, regardless of whether we are host or client
            chunkManager.deleteItemPickup(packetData.pickupDeleted);
        }
        else if (packet.type == PacketType::InventoryAddItem)
        {
            PacketDataInventoryAddItem packetData;
            packetData.deserialise(packet.data);
            inventory.addItem(packetData.itemType, packetData.amount, true);
        }
        else if (packet.type == PacketType::ChunkRequests && isLobbyHost)
        {
            PacketDataChunkRequests packetData;
            packetData.deserialise(packet.data);
            handleChunkRequestsFromClient(packetData, messages[i]->m_identityPeer);
        }
        else if (packet.type == PacketType::ChunkDatas && !isLobbyHost)
        {
            PacketDataChunkDatas packetData;
            packetData.deserialise(packet.data);
            handleChunkDatasFromHost(packetData);
        }

        messages[i]->Release();
    }
}

void Game::sendHostMessages()
{
    if (!isLobbyHost)
    {
        return;
    }

    uint64_t steamID = SteamUser()->GetSteamID().ConvertToUint64();

    // Send world info
    PacketDataWorldInfo worldInfoData;
    worldInfoData.gameTime = gameTime;
    worldInfoData.day = dayCycleManager.getCurrentDay();
    worldInfoData.time = dayCycleManager.getCurrentTime();
    worldInfoData.setHostPingLocation();
    Packet worldInfoPacket;
    worldInfoPacket.set(worldInfoData);

    for (auto& client : networkPlayers)
    {
        SteamNetworkingIdentity clientIdentity;
        clientIdentity.SetSteamID64(client.first);
        worldInfoPacket.sendToUser(clientIdentity, k_nSteamNetworkingSend_Unreliable, 0);
    }

    std::unordered_map<uint64_t, Packet> playerInfoPackets;
    playerInfoPackets[steamID] = Packet();
    playerInfoPackets[steamID].set(player.getNetworkPlayerInfo(steamID));

    // Get player infos
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        playerInfoPackets[iter->first] = Packet();
        playerInfoPackets[iter->first].set(iter->second.getNetworkPlayerInfo(iter->first));
    }

    // Send player info
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        SteamNetworkingIdentity identity;
        identity.SetSteamID64(iter->first);

        for (auto subIter = networkPlayers.begin(); subIter != networkPlayers.end(); subIter++)
        {
            // Don't send player their own info
            if (iter == subIter)
            {
                continue;
            }

            playerInfoPackets[subIter->first].sendToUser(identity, k_nSteamNetworkingSend_Unreliable, 0);
        }
        
        // Send host player data
        playerInfoPackets[steamID].sendToUser(identity, k_nSteamNetworkingSend_Unreliable, 0);
    }
}

EResult Game::sendPacketToClients(const Packet& packet, int nSendFlags, int nRemoteChannel)
{
    if (!isLobbyHost)
    {
        return EResult::k_EResultAccessDenied;
    }

    EResult result = EResult::k_EResultOK;
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        SteamNetworkingIdentity identity;
        identity.SetSteamID64(iter->first);
        EResult sendResult = packet.sendToUser(identity, nSendFlags, nRemoteChannel);

        if (sendResult != EResult::k_EResultOK)
        {
            result = sendResult;
        }
    }

    return result;
}

void Game::handleChunkRequestsFromClient(const PacketDataChunkRequests& chunkRequests, const SteamNetworkingIdentity& client)
{
    if (!isLobbyHost)
    {
        return;
    }

    PacketDataChunkDatas packetChunkDatas;

    const char* steamName = SteamFriends()->GetFriendPersonaName(client.GetSteamID());
    
    int minChunkX = 9999999;
    int minChunkY = 9999999;
    int maxChunkX = -9999999;
    int maxChunkY = -9999999;
    
    for (ChunkPosition chunk : chunkRequests.chunkRequests)
    {
        packetChunkDatas.chunkDatas.push_back(chunkManager.getChunkDataAndGenerate(chunk, *this));
        minChunkX = std::min((int)chunk.x, minChunkX);
        minChunkY = std::min((int)chunk.y, minChunkY);
        maxChunkX = std::max((int)chunk.x, maxChunkX);
        maxChunkY = std::max((int)chunk.y, maxChunkY);
    }

    Packet packet;
    packet.set(packetChunkDatas, true);
    
    printf(("Sending " + std::to_string(chunkRequests.chunkRequests.size()) + " chunks in range (" + std::to_string(minChunkX) + ", " + std::to_string(minChunkY) +
        ") to (" + std::to_string(maxChunkX) + ", " + std::to_string(maxChunkY) + ") to " + steamName + " (size: " + std::to_string(packet.getSize()) +
        " bytes, uncompressed: " + std::to_string(packet.getUncompressedSize()) + " bytes, ratio: " + std::to_string(packet.getCompressionRatio()) + ")\n").c_str());

    packet.sendToUser(client, k_nSteamNetworkingSend_Reliable, 0);
}

void Game::sendClientMessages()
{
    if (isLobbyHost)
    {
        return;
    }

    uint64_t steamID = SteamUser()->GetSteamID().ConvertToUint64();

    Packet packet;
    packet.set(player.getNetworkPlayerInfo(steamID));

    SteamNetworkingIdentity hostIdentity;
    hostIdentity.SetSteamID64(lobbyHost);

    packet.sendToUser(hostIdentity, k_nSteamNetworkingSend_Unreliable, 0);
}

EResult Game::sendPacketToHost(const Packet& packet, int nSendFlags, int nRemoteChannel)
{
    if (isLobbyHost)
    {
        return EResult::k_EResultAccessDenied;
    }

    SteamNetworkingIdentity hostIdentity;
    hostIdentity.SetSteamID64(lobbyHost);

    return packet.sendToUser(hostIdentity, nSendFlags, nRemoteChannel);
}

void Game::handleChunkDatasFromHost(const PacketDataChunkDatas& chunkDatas)
{
    if (isLobbyHost)
    {
        return;
    }

    for (const auto& chunkData : chunkDatas.chunkDatas)
    {
        chunkManager.setChunkData(chunkData, *this);
        
        if (chunkRequestsOutstanding.contains(chunkData.chunkPosition))
        {
            chunkRequestsOutstanding.erase(chunkData.chunkPosition);    
        }

        printf(("Received chunk (" + std::to_string(chunkData.chunkPosition.x) + ", " + std::to_string(chunkData.chunkPosition.y) + ") data from host\n").c_str());
    }
}

void Game::requestChunksFromHost(std::vector<ChunkPosition>& chunks)
{
    if (!multiplayerGame || isLobbyHost)
    {
        return;
    }

    for (auto iter = chunks.begin(); iter != chunks.end();)
    {
        if (!chunkRequestsOutstanding.contains(*iter))
        {
            chunkRequestsOutstanding[*iter] = gameTime;
        }
        else if (gameTime - chunkRequestsOutstanding.at(*iter) >= CHUNK_REQUEST_OUTSTANDING_MAX_TIME)
        {
            // Reset time and request again
            chunkRequestsOutstanding[*iter] = gameTime;
        }
        else
        {
            // Chunk is still being requested - do not request again (yet)
            iter = chunks.erase(iter);
            continue;
        }

        iter++;
    }

    // Do not request 0 chunks
    if (chunks.size() <= 0)
    {
        return;
    }
    
    printf(("Requesting " + std::to_string(chunks.size()) + " chunks from host\n").c_str());

    PacketDataChunkRequests packetData;
    packetData.chunkRequests = chunks;
    Packet packet;
    packet.set(packetData);
    sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
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

        #if (!RELEASE_BUILD)
        if (event.key.code == sf::Keyboard::F1)
        {
            DebugOptions::debugOptionsMenuOpen = !DebugOptions::debugOptionsMenuOpen;
            return;
        }
        #endif
    }

    // ImGui
    ImGui::SFML::ProcessEvent(window, event);
}

void Game::handleSDLEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        InputManager::processEvent(event);
    }
}

void Game::toggleFullScreen()
{
    fullScreen = !fullScreen;

    sf::VideoMode videoMode = sf::VideoMode::getDesktopMode();

    unsigned int windowStyle = sf::Style::Default;
    if (fullScreen) windowStyle = sf::Style::None;

    SDL_DestroyWindow(sdlWindow);
    
    window.create(videoMode, GAME_TITLE, windowStyle);

    sdlWindow = SDL_CreateWindowFrom(reinterpret_cast<void*>(window.getSystemHandle()));

    // Set window stuff
    window.setIcon(256, 256, icon.getPixelsPtr());
    window.setFramerateLimit(165);
    window.setVerticalSyncEnabled(true);
    window.setMouseCursorVisible(false);
    window.setJoystickSensorEventsEnabled(false);

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
    sf::IntRect textureRect(80, 32, 8, 8);
    sf::Vector2f textureCentreRatio;

    bool shiftMode = InputManager::isActionActive(InputAction::UI_SHIFT);

    bool canQuickTransfer = InventoryGUI::canQuickTransfer(mouseScreenPos, shiftMode, inventory, chestDataPool.getChestDataPtr(openedChestID));

    if (InputManager::isControllerActive())
    {
        if (gameState != GameState::MainMenu && (worldMenuState == WorldMenuState::Main ||
            worldMenuState == WorldMenuState::Inventory || worldMenuState == WorldMenuState::NPCShop))
        {
            if (canQuickTransfer)
            {
                textureRect = sf::IntRect(96, 48, 15, 15);
                textureCentreRatio = sf::Vector2f(1.0f / 3.0f, 1.0f / 3.0f);
            }
            else if (InputManager::isActionActive(InputAction::USE_TOOL))
            {
                textureRect = sf::IntRect(80, 64, 8, 8);
                textureCentreRatio = sf::Vector2f(0.5f, 0.5f);
            }
            else if (InputManager::isActionActive(InputAction::INTERACT))
            {
                textureRect = sf::IntRect(96, 64, 10, 10);
                textureCentreRatio = sf::Vector2f(0.5f, 0.5f);
            }
            else
            {
                textureRect = sf::IntRect(80, 48, 10, 10);
                textureCentreRatio = sf::Vector2f(0.5f, 0.5f);
            }
        }
        else
        {
            return;
        }
    }
    else
    {
        mouseScreenPos.x = std::max(std::min(mouseScreenPos.x, static_cast<float>(window.getSize().x)), 0.0f);
        mouseScreenPos.y = std::max(std::min(mouseScreenPos.y, static_cast<float>(window.getSize().y)), 0.0f);

        // Switch mouse cursor mode
        if (canQuickTransfer)
        {
            textureRect = sf::IntRect(96, 32, 12, 12);
        }
    }

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    TextureManager::drawSubTexture(window, {TextureType::UI, mouseScreenPos, 0, {3 * intScale, 3 * intScale}, textureCentreRatio}, textureRect);
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

    sf::Vector2f resolution = static_cast<sf::Vector2f>(ResolutionHandler::getResolution());
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    static constexpr int GLYPH_SPACING = 50;
    static constexpr int GLYPH_X_PADDING = 70;

    for (int i = 0; i < actionStrings.size(); i++)
    {
        auto actionString = actionStrings[i];

        std::optional<ControllerGlyph> glyph = InputManager::getBoundActionControllerGlyph(actionString.first);

        // Set to no bind glyph by default
        sf::IntRect glyphTextureRect(192, 192, 16, 16);

        if (glyph.has_value())
        {
            glyphTextureRect.left = buttonGlyphXOffset.at(glyph.value());
            glyphTextureRect.top = 192 + InputManager::getGlyphType() * 16;
        }

        // Draw button glyph
        TextureDrawData glyphDrawData;
        glyphDrawData.type = TextureType::UI;
        glyphDrawData.position = sf::Vector2f(resolution.x - GLYPH_X_PADDING / 2.0f * intScale, resolution.y - (i + 1) * GLYPH_SPACING * intScale);
        glyphDrawData.centerRatio = sf::Vector2f(0.5f, 0.5f);
        glyphDrawData.scale = sf::Vector2f(3, 3) * intScale;

        spriteBatch.draw(window, glyphDrawData, glyphTextureRect);

        // Draw action text
        TextDrawData textDrawData;
        textDrawData.text = actionString.second;
        textDrawData.position = sf::Vector2f(resolution.x, resolution.y - (i + 1) * GLYPH_SPACING * intScale);
        textDrawData.size = 24 * intScale;
        textDrawData.colour = sf::Color(255, 255, 255);
        textDrawData.outlineColour = sf::Color(46, 34, 47);
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

    if (!multiplayerGame || isLobbyHost)
    {
        // ImGui::Text("Save / Load");
    
        if (ImGui::Button("Save"))
        {
            saveGame();
        }
    
        // if (ImGui::Button("Load"))
        // {
        //     loadGame(currentSaveFileSummary);
        // }
    }

    ImGui::Spacing();

    ImGui::Checkbox("Smooth Lighting", &smoothLighting);

    float time = dayCycleManager.getCurrentTime();
    if (!multiplayerGame || isLobbyHost)
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

    ImGui::Text(("Weather value: " + std::to_string(weatherSystem.sampleWeatherFunction(gameTime))).c_str());
    ImGui::Text(("Weather transition: " + std::to_string(weatherSystem.getDestinationTransitionProgress())).c_str());

    ImGui::End();   
}
#endif