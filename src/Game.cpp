#include "Game.hpp"

// PRIORITY: HIGH
// TODO: Structure functionality (item spawns, crafting stations etc)
// TODO: Different types of structures
// TODO: Different types of tools? (fishing rod etc)

// PRIORITY: LOW
// TODO: Prevent chests from being destroyed when containing items

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

    chunkManager.setSeed(rand());
    chunkManager.setWorldSize(240);
    chunkManager.setPlanetType(PlanetGenDataLoader::getPlanetTypeFromName("Earthlike"));

    // Create noise
    // noise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
    // noise.SetSeed(rand());
    // noise.SetFrequency(0.1);

    // Initialise values
    gameTime = 0;
    gameState = GameState::OnPlanet;
    destinationGameState = gameState;
    transitionGameStateTimer = 0.0f;
    worldMenuState = WorldMenuState::Main;

    openedChestID = 0xFFFF;

    musicTypePlaying = std::nullopt;
    musicGapTimer = 0.0f;
    musicGap = 0.0f;

    inventory = InventoryData(32);

    // Initialise day/night cycle
    dayNightToggleTimer = 0.0f;
    worldDarkness = 0.0f;
    isDay = true;

    // Initialise GUI
    InventoryGUI::initialise(inventory);

    generateWaterNoiseTexture();

    // Find valid player spawn
    sf::Vector2f spawnPos = chunkManager.findValidSpawnPosition(2);
    player.setPosition(spawnPos);

    // Initialise inventory
    giveStartingInventory();

    Camera::instantUpdate(player.getPosition());

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
    // waterShader->setUniform("waterColor", sf::Glsl::Vec4(77 / 255.0f, 155 / 255.0f, 230 / 255.0f, 1.0f));
}

void Game::giveStartingInventory()
{
    inventory.addItem(ItemDataLoader::getItemTypeFromName("Wooden Pickaxe"), 1);

    changePlayerTool();
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

void Game::run()
{
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        gameTime += dt;

        SteamAPI_RunCallbacks();
        ImGui::SFML::Update(window, sf::seconds(dt));

        window.setView(view);

        switch (gameState)
        {
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

        drawDebugMenu(dt);

        if (ImGui::GetIO().WantCaptureMouse)
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        }
        else
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        }

        ImGui::SFML::Render(window);

        window.display();
    }
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

    ImGui::End();   
}

void Game::runOnPlanet(float dt)
{
    sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    bool shiftMode = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

    // Handle events
    for (auto event = sf::Event{}; window.pollEvent(event);)
    {
        handleEventsWindow(event);

        if (isStateTransitioning())
            continue;

        if (event.type == sf::Event::KeyPressed)
        {
            if (worldMenuState == WorldMenuState::Main)
            {
                if (event.key.code == sf::Keyboard::E)
                {
                    worldMenuState = WorldMenuState::Inventory;
                    closeChest();
                }
            }
            else
            {
                if (event.key.code == sf::Keyboard::E && worldMenuState == WorldMenuState::Inventory)
                {
                    InventoryGUI::handleClose(inventory, chestDataPool.getChestDataPtr(openedChestID));
                    worldMenuState = WorldMenuState::Main;
                    closeChest();
                }

                if (event.key.code == sf::Keyboard::Escape)
                {
                    InventoryGUI::handleClose(inventory, chestDataPool.getChestDataPtr(openedChestID));
                    worldMenuState = WorldMenuState::Main;
                    closeChest();
                }
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
                        }
                        else
                        {
                            changePlayerTool();
                        }
                        break;
                    }
                    case WorldMenuState::Inventory:
                        if (!InventoryGUI::isMouseOverUI(mouseScreenPos))
                        {
                            attemptUseTool();
                            attemptBuildObject();
                            attemptPlaceLand();
                        }
                        InventoryGUI::handleLeftClick(mouseScreenPos, shiftMode, inventory, chestDataPool.getChestDataPtr(openedChestID));
                        changePlayerTool();
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
                        if (InventoryGUI::isMouseOverUI(mouseScreenPos))
                        {
                            InventoryGUI::handleRightClick(mouseScreenPos, shiftMode, inventory, chestDataPool.getChestDataPtr(openedChestID));
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

    // Inventory GUI updating
    InventoryGUI::updateItemPopups(dt);

    if (worldMenuState == WorldMenuState::Main)
    {
        InventoryGUI::updateHotbar(dt, mouseScreenPos);
    }
    else if (worldMenuState == WorldMenuState::Inventory)
    {
        // Update inventory GUI available recipes if required, and animations
        InventoryGUI::updateAvailableRecipes(inventory, nearbyCraftingStationLevels);
        InventoryGUI::updateInventory(mouseScreenPos, dt, chestDataPool.getChestDataPtr(openedChestID));
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

    switch (worldMenuState)
    {
        case WorldMenuState::Main:
            InventoryGUI::drawHotbar(window, mouseScreenPos, inventory);
            InventoryGUI::drawItemPopups(window);
            break;
        
        case WorldMenuState::Inventory:
            InventoryGUI::draw(window, gameTime, mouseScreenPos, inventory, chestDataPool.getChestDataPtr(openedChestID));
            break;
    }

    drawMouseCursor();
}

void Game::updateOnPlanet(float dt)
{
    sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    // updateDayNightCycle(dt);

    int worldSize = chunkManager.getWorldSize();

    // Update camera
    Camera::update(player.getPosition(), mouseScreenPos, dt);

    // Update cursor
    ObjectType objectType = InventoryGUI::getHeldObjectType();
    if (objectType <= 0) objectType = InventoryGUI::getHotbarSelectedObject(inventory);

    Cursor::updateTileCursor(window, dt, worldSize, chunkManager, player.getCollisionRect(), objectType, player.getTool());

    // Cursor enable / disable
    if (InventoryGUI::getHeldToolType() < 0)
    {
        if (InventoryGUI::heldItemPlacesLand() || InventoryGUI::hotbarItemPlacesLand(inventory))
            Cursor::setCursorPlacingLand(window);
    }

    // Enable / disable cursor drawing depending on player reach
    // if (player.getTool() >= 0)
    Cursor::setCursorHidden(!player.canReachPosition(Cursor::getMouseWorldPos(window)));
    

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
    }

    // Update (loaded) chunks
    chunkManager.updateChunks();
    chunkManager.updateChunksObjects(dt);
    chunkManager.updateChunksEntities(dt);
    
    // Get nearby crafting stations
    nearbyCraftingStationLevels = chunkManager.getNearbyCraftingStationLevels(player.getChunkInside(worldSize), player.getChunkTileInside(worldSize), 4);

    if (!isStateTransitioning())
        testEnterStructure();

    // Close chest if out of range
    checkChestOpenInRange();
}

void Game::drawOnPlanet(float dt)
{
    // Get world objects
    std::vector<WorldObject*> worldObjects = chunkManager.getChunkObjects();
    std::vector<WorldObject*> entities = chunkManager.getChunkEntities();
    worldObjects.insert(worldObjects.end(), entities.begin(), entities.end());
    worldObjects.push_back(&player);

    drawWorld(dt, worldObjects, entities);
    drawLighting(dt, worldObjects, entities);


    // UI
    Cursor::drawCursor(window);

    if (InventoryGUI::getHeldToolType() < 0)
    {
        ObjectType placeObject = InventoryGUI::getHeldObjectType();
        if (placeObject < 0)
        {
            placeObject = InventoryGUI::getHotbarSelectedObject(inventory);
        }

        if (placeObject >= 0)
        {
            drawGhostPlaceObjectAtCursor(placeObject);
        }

        // Draw land to place if held
        if ((InventoryGUI::heldItemPlacesLand() || InventoryGUI::hotbarItemPlacesLand(inventory)))
        {
            drawGhostPlaceLandAtCursor();
        }
    }
}

void Game::updateInStructure(float dt)
{
    sf::Vector2f mouseScreenPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    const Room& structureRoom = structureRoomPool.getRoom(structureEnteredID);

    if (!isStateTransitioning())
        player.updateInStructure(dt, Cursor::getMouseWorldPos(window), structureRoom);

    Camera::update(player.getPosition(), mouseScreenPos, dt);

    // Continue to update objects and entities in world
    chunkManager.updateChunksObjects(dt);
    chunkManager.updateChunksEntities(dt);

    if (!isStateTransitioning())
        testExitStructure();
}

void Game::drawInStructure(float dt)
{
    const Room& structureRoom = structureRoomPool.getRoom(structureEnteredID);
    structureRoom.draw(window);

    player.draw(window, spriteBatch, dt, gameTime, chunkManager.getWorldSize(), sf::Color(255, 255, 255));
}

void Game::updateMusic(float dt)
{
    // Music playing
    if (musicTypePlaying.has_value())
    {
        if (Sounds::isMusicFinished(musicTypePlaying.value()))
        {
            musicTypePlaying = std::nullopt;
        }
        else
        {
            return;
        }
    }
    
    musicGapTimer += dt;

    if (musicGapTimer < musicGap)
        return;
    
    // Play new music as music gap has ended
    static constexpr std::array<MusicType, 2> musicTypes = {MusicType::WorldTheme, MusicType::WorldTheme2};
    int musicTypeChance = rand() % musicTypes.size();

    musicTypePlaying = musicTypes[musicTypeChance];
    Sounds::playMusic(musicTypePlaying.value(), 70.0f);

    musicGapTimer = 0.0f;
    musicGap = MUSIC_GAP_MIN + rand() % 5;
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

void Game::attemptUseTool()
{
    if (player.getTool() < 0)
        return;

    if (player.isUsingTool())
        return;
    
    if (InventoryGUI::getHeldObjectType() >= 0 || InventoryGUI::heldItemPlacesLand())
        return;

    // Get mouse position in screen space and world space
    sf::Vector2f mouseWorldPos = Cursor::getMouseWorldPos(window);

    player.useTool();

    if (gameState != GameState::OnPlanet)
        return;

    if (!player.canReachPosition(mouseWorldPos))
        return;
    
    // Get current tool damage amount
    ToolType currentTool = player.getTool();

    const ToolData& toolData = ToolDataLoader::getToolData(currentTool);

    Entity* selectedEntity = chunkManager.getSelectedEntity(Cursor::getSelectedChunk(chunkManager.getWorldSize()), mouseWorldPos);
    if (selectedEntity != nullptr)
    {
        selectedEntity->damage(toolData.damage, inventory);
    }
    else
    {
        bool canDestroyObject = chunkManager.canDestroyObject(Cursor::getSelectedChunk(chunkManager.getWorldSize()),
                                                        Cursor::getSelectedChunkTile(),
                                                        player.getCollisionRect());

        if (!canDestroyObject)
            return;

        std::optional<BuildableObject>& selectedObjectOptional = chunkManager.getChunkObject(Cursor::getSelectedChunk(
            chunkManager.getWorldSize()), Cursor::getSelectedChunkTile());

        if (selectedObjectOptional.has_value())
        {
            BuildableObject& selectedObject = selectedObjectOptional.value();
            bool destroyed = selectedObject.damage(toolData.damage, inventory);

            if (destroyed)
            {
                if (selectedObject.getChestCapactity() > 0)
                {
                    removeChestFromData(selectedObject);
                }
            }
        }
    }
}

void Game::changePlayerTool()
{
    // Get currently selected tool in inventory and hotbar
    ToolType hotbarTool = InventoryGUI::getHotbarSelectedTool(inventory);
    ToolType inventoryTool = InventoryGUI::getHeldToolType();

    // Get tool currently held by player

    // If inventory tool is selected, override hotbar tool
    if (inventoryTool >= 0)
    {
        player.setTool(inventoryTool);
    }
    else if (hotbarTool >= 0)
    {
        // Set tool from hotbar, as not selected from inventory
        player.setTool(hotbarTool);
    }
    else
    {
        player.setTool(-1);
    }
}

void Game::attemptObjectInteract()
{
    if (gameState != GameState::OnPlanet)
        return;

    // Get mouse position in screen space and world space
    sf::Vector2f mouseWorldPos = Cursor::getMouseWorldPos(window);

    if (!player.canReachPosition(mouseWorldPos))
        return;
    
    std::optional<BuildableObject>& selectedObjectOptional = chunkManager.getChunkObject(
        Cursor::getSelectedChunk(chunkManager.getWorldSize()), Cursor::getSelectedChunkTile());
    
    if (selectedObjectOptional.has_value())
    {
        BuildableObject& selectedObject = selectedObjectOptional.value();

        ObjectInteractionEventData interactionEvent = selectedObject.interact();
        if (interactionEvent.interactionType == ObjectInteraction::Chest)
        {
            // If chest has ID 0xFFFF, then has not been initialised
            // Therefore must initialise chest
            if (selectedObject.getChestID() == 0xFFFF)
            {
                initChestInData(selectedObject);
            }

            // Close chest if currently open is selected
            if (selectedObject.getChestID() == openedChestID)
            {
                closeChest();
            }
            else
            {
                // If a chest is currently open, close it
                if (openedChestID != 0xFFFF)
                {
                    closeChest();
                }

                openedChestID = selectedObject.getChestID();
                openedChest.chunk = selectedObject.getChunkInside(chunkManager.getWorldSize());
                openedChest.tile = selectedObject.getChunkTileInside(chunkManager.getWorldSize());
                openedChestPos = selectedObject.getPosition();

                // Reset chest UI animations as opened new chest
                InventoryGUI::chestOpened(chestDataPool.getChestDataPtr(openedChestID));

                // Tell chest object it has been opened
                selectedObject.openChest();

                // Open inventory to see chest
                worldMenuState = WorldMenuState::Inventory;
            }
        }
    }
}

void Game::attemptBuildObject()
{
    if (gameState != GameState::OnPlanet)
        return;

    ObjectType objectType = InventoryGUI::getHeldObjectType();
    bool placeFromHotbar = false;

    // Do not build if holding tool in inventory (not in hotbar)
    if (InventoryGUI::getHeldToolType() >= 0)
        return;

    // If object not picked up from inventory, check hotbar
    if (objectType <= 0)
    {
        objectType = InventoryGUI::getHotbarSelectedObject(inventory);
        placeFromHotbar = true;
    }

    if (objectType < 0)
        return;

    // bool canAfford = Inventory::canBuildObject(objectType);
    bool canPlace = chunkManager.canPlaceObject(Cursor::getSelectedChunk(chunkManager.getWorldSize()),
                                                Cursor::getSelectedChunkTile(),
                                                objectType,
                                                player.getCollisionRect());

    bool inRange = player.canReachPosition(Cursor::getMouseWorldPos(window));

    if (canPlace && inRange)
    {
        // Remove object from being held
        if (placeFromHotbar)
        {
            InventoryGUI::placeHotbarObject(inventory);
        }
        else
        {
            InventoryGUI::placeHeldObject();
        }

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

    // Do not build if holding tool in inventory (not in hotbar)
    if (InventoryGUI::getHeldToolType() >= 0)
        return;

    bool placeFromHotbar = false;

    if (!InventoryGUI::heldItemPlacesLand())
    {
        if (InventoryGUI::hotbarItemPlacesLand(inventory))
        {
            placeFromHotbar = true;
        }
        else
        {
            return;
        }
    }
    
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
    if (placeFromHotbar)
    {
        InventoryGUI::placeHotbarObject(inventory);
    }
    else
    {
        InventoryGUI::placeHeldObject();
    }
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
    
    BuildableObject objectGhost(Cursor::getLerpedSelectPos() + sf::Vector2f(TILE_SIZE_PIXELS_UNSCALED / 2.0f, TILE_SIZE_PIXELS_UNSCALED / 2.0f), object);

    objectGhost.draw(window, spriteBatch, 0.0f, 0, chunkManager.getWorldSize(), drawColor);

    spriteBatch.endDrawing(window);
}

void Game::drawGhostPlaceLandAtCursor()
{
    sf::Vector2f tileWorldPosition = Cursor::getLerpedSelectPos();

    // Change color depending on whether can place land or not
    sf::Color landGhostColor(255, 0, 0, 180);
    if (chunkManager.canPlaceLand(Cursor::getSelectedChunk(chunkManager.getWorldSize()), Cursor::getSelectedChunkTile()))
    {
        landGhostColor = sf::Color(0, 255, 0, 180);
    }

    float scale = ResolutionHandler::getScale();

    // TODO: In future, change texture rect used depending on world
    sf::IntRect textureRect(32, 0, 16, 16);

    // Draw tile at screen position
    TextureManager::drawSubTexture(window, {
        .type = TextureType::GroundTiles,
        .position = Camera::worldToScreenTransform(tileWorldPosition),
        .scale = {scale, scale},
        .colour = landGhostColor
    }, textureRect);
}

void Game::drawWorld(float dt, std::vector<WorldObject*>& worldObjects, std::vector<WorldObject*>& entities)
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

    spriteBatch.endDrawing(worldTexture);

    worldTexture.display();
}

void Game::drawLighting(float dt, std::vector<WorldObject*>& worldObjects, std::vector<WorldObject*>& entities)
{
    // Draw light sources on light texture
    sf::RenderTexture lightTexture;
    lightTexture.create(window.getSize().x, window.getSize().y);

    unsigned char ambientRedLight = (1.f - worldDarkness) * 255.0f;
    unsigned char ambientGreenLight = (1.f - worldDarkness * 0.97f) * 255.0f;
    unsigned char ambientBlueLight = (1.f - worldDarkness * 0.93f) * 255.0f;

    lightTexture.clear({ambientRedLight, ambientGreenLight, ambientBlueLight, 255});

    player.drawLightMask(lightTexture);

    for (WorldObject* entity : entities)
    {
        Entity* entityCasted = static_cast<Entity*>(entity);
        entityCasted->drawLightMask(lightTexture);
    }

    lightTexture.display();

    sf::Sprite lightTextureSprite(lightTexture.getTexture());
    // lightTextureSprite.setColor(sf::Color(255, 255, 255, 255));

    worldTexture.draw(lightTextureSprite, sf::BlendMultiply);

    worldTexture.display();

    sf::Sprite worldTextureSprite(worldTexture.getTexture());
    window.draw(worldTextureSprite);
}

void Game::initChestInData(BuildableObject& chest)
{
    int chestCapacity = chest.getChestCapactity();

    uint16_t chestID = chestDataPool.createChest(chestCapacity);

    if (chestID == 0xFFFF)
        return;
    
    chest.setChestID(chestID);
}

void Game::removeChestFromData(BuildableObject& chest)
{
    chestDataPool.destroyChest(chest.getChestID());

    if (openedChestID == chest.getChestID())
    {
        closeChest();
    }
}

void Game::checkChestOpenInRange()
{
    if (openedChestID == 0xFFFF)
        return;
    
    std::optional<BuildableObject>& chestObject = chunkManager.getChunkObject(openedChest.chunk, openedChest.tile);

    if (!chestObject.has_value())
    {
        closeChest();
        return;
    }

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
    
    // Tell chest object it has been closed
    std::optional<BuildableObject>& chestObject = chunkManager.getChunkObject(openedChest.chunk, openedChest.tile);
    if (chestObject.has_value())
    {
        chestObject->closeChest();
    }

    openedChestID = 0xFFFF;
    openedChest.chunk = ChunkPosition(0, 0);
    openedChest.tile = sf::Vector2i(0, 0);
    openedChestPos = sf::Vector2f(0, 0);
}

void Game::testEnterStructure()
{
    StructureEnterEvent enterEvent;
    if (!chunkManager.isPlayerInStructureEntrance(player.getPosition(), enterEvent))
        return;
    
    // Structure has been entered

    // Create room data
    if (enterEvent.enteredStructure->getStructureID() == 0xFFFFFFFF)
    {
        structureEnteredID = structureRoomPool.createRoom(enterEvent.enteredStructure->getStructureType());
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
            // Exit structure
            structureEnteredID = 0xFFFFFFFF;

            player.setPosition(structureEnteredPos);
            Camera::instantUpdate(player.getPosition());
            break;
        }
    }

    gameState = newState;
}

void Game::updateDayNightCycle(float dt)
{
    // Update day / night cycle
    dayNightToggleTimer += dt;

    if (dayNightToggleTimer >= 15.0f)
    {
        dayNightToggleTimer = 0.0f;
        if (isDay) floatTween.startTween(&worldDarkness, 0.0f, 0.95f, 7, TweenTransition::Sine, TweenEasing::EaseInOut);
        else floatTween.startTween(&worldDarkness, 0.95f, 0.0f, 7, TweenTransition::Sine, TweenEasing::EaseInOut);
        isDay = !isDay;
    }
}