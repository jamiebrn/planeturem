#include "GUI/MainMenuGUI.hpp"
#include "Game.hpp"

void MainMenuGUI::initialise()
{
    canInteract = true;
    mainMenuState = MainMenuState::Main;

    deleteSaveHoldTime = 0.0f;
    deletingSaveIndex = -1;

    deferHoverRectReset = false;

    // static const std::string backgroundWorldSeed = "Planeturem";
    menuWorldData.chunkManager.setSeed(rand());
    menuWorldData.chunkManager.setPlanetType(0);

    int worldSize = menuWorldData.chunkManager.getWorldSize();
    ChunkPosition worldViewChunk = menuWorldData.chunkManager.findValidSpawnChunk(2);
    worldViewPosition.x = worldViewChunk.x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    worldViewPosition.y = worldViewChunk.y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;

    menuCamera.instantUpdate(worldViewPosition);
}

void MainMenuGUI::initialisePauseMenu()
{
    pauseMenuState = PauseMenuState::Main;
    // showPauseMenuWishlist = false;
    resetHoverRect();
}

void MainMenuGUI::update(float dt, pl::Vector2f mouseScreenPos, Game& game)
{
    // Update background world / chunk manager
    worldViewPosition.x += 20.0f * dt;
    worldViewPosition.y += 30.0f * dt;

    menuCamera.update(worldViewPosition, pl::Vector2f(0, 0), dt);

    int worldPixelSize = menuWorldData.chunkManager.getWorldSize() * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;

    pl::Vector2f positionBeforeWrap = worldViewPosition;
    worldViewPosition.x = fmod(fmod(worldViewPosition.x, worldPixelSize) + worldPixelSize, worldPixelSize);
    worldViewPosition.y = fmod(fmod(worldViewPosition.y, worldPixelSize) + worldPixelSize, worldPixelSize);

    if (worldViewPosition != positionBeforeWrap)
    {
        menuCamera.handleWorldWrap(worldViewPosition - positionBeforeWrap);
    }

    menuWorldData.chunkManager.updateChunks(game, 1.0f, {menuCamera.getChunkViewRange()}, nullptr);
    menuWorldData.chunkManager.updateChunksObjects(game, dt, 0.0f);
    menuWorldData.chunkManager.updateChunksEntities(dt, menuWorldData.projectileManager, game, false);
}

std::optional<MainMenuEvent> MainMenuGUI::createAndDraw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, float dt, float applicationTime)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    pl::Vector2f resolution = static_cast<pl::Vector2f>(ResolutionHandler::getResolution());
    
    // Draw background chunks / world
    std::vector<WorldObject*> worldObjects = menuWorldData.chunkManager.getChunkObjects(menuCamera.getChunkViewRange());
    std::vector<WorldObject*> entities = menuWorldData.chunkManager.getChunkEntities(menuCamera.getChunkViewRange());
    worldObjects.insert(worldObjects.end(), entities.begin(), entities.end());

    pl::Framebuffer worldTexture;
    game.drawWorld(worldTexture, dt, worldObjects, menuWorldData, menuCamera);

    pl::VertexArray worldRect;
    worldRect.addQuad(pl::Rect<float>(0, 0, worldTexture.getWidth(), worldTexture.getHeight()), pl::Color(),
        pl::Rect<float>(0, worldTexture.getHeight(), worldTexture.getWidth(), -worldTexture.getHeight()));
    
    window.draw(worldRect, *Shaders::getShader(ShaderType::Default), &worldTexture.getTexture(), pl::BlendMode::Alpha);

    drawPanel(window);

    int scaledPanelPaddingX = getScaledPanelPaddingX();

    // Draw title
    pl::DrawData titleDrawData;
    titleDrawData.texture = TextureManager::getTexture(TextureType::UI);
    titleDrawData.shader = Shaders::getShader(ShaderType::Default);
    titleDrawData.scale = pl::Vector2f(3, 3) * intScale;
    titleDrawData.position = pl::Vector2f(scaledPanelPaddingX + panelWidth / 2 * intScale, std::round((140 + std::sin(applicationTime) * 20) * intScale));
    titleDrawData.centerRatio = pl::Vector2f(0.5f, 0.5f);
    titleDrawData.textureRect = pl::Rect<int>(21, 160, 212, 32);

    TextureManager::drawSubTexture(window, titleDrawData);

    MainMenuState nextUIState = mainMenuState;

    const int startElementYPos = resolution.y * 0.33f;
    int elementYPos = startElementYPos;

    std::optional<MainMenuEvent> menuEvent = std::nullopt;

    int buttonTextSize = 24 * intScale;

    // Buttons / UI
    switch (mainMenuState)
    {
        case MainMenuState::Main:
        {
            if (const Button& button = guiContext.createButton(
                scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, buttonTextSize, "New", buttonStyle);
                button.isClicked())
            {
                saveNameInput = "";
                worldSeedInput = "";
                newGamePage = 0;
                selectedBodyColor = pl::Color(158, 69, 57);
                selectedBodyColorValueHSV = 100.0f;
                selectedSkinColor = pl::Color(230, 144, 78);
                selectedSkinColorValueHSV = 100.0f;
                nextUIState = MainMenuState::StartingNew;
            }

            elementYPos += 100 * intScale;

            if (const Button& button = guiContext.createButton(
                scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, buttonTextSize, "Load", buttonStyle);
                button.isClicked())
            {
                nextUIState = MainMenuState::SelectingLoad;
                
                GameSaveIO io;
                saveFileSummaries = io.getSaveFiles();

                saveFilePage = 0;
            }

            elementYPos += 100 * intScale;

            if (const Button& button = guiContext.createButton(
                scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, buttonTextSize, "Options", buttonStyle);
                button.isClicked())
            {
                nextUIState = MainMenuState::Options;
                optionsPage = 0;
            }

            elementYPos += 100 * intScale;

            if (const Button& button = guiContext.createButton(
                scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, buttonTextSize, "Exit", buttonStyle);
                button.isClicked())
            {
                menuEvent = MainMenuEvent();
                menuEvent->type = MainMenuEventType::Quit;
            }
            break;
        }
        case MainMenuState::StartingNew:
        {
            if (newGamePage == 0)
            {
                guiContext.createTextEnter(scaledPanelPaddingX, elementYPos,
                    panelWidth * intScale, 75 * intScale, 20 * intScale, "Save Name", &saveNameInput, panelWidth / 5 * intScale, 30 * intScale, 30);
    
                elementYPos += 100 * intScale;
    
                guiContext.createTextEnter(scaledPanelPaddingX, elementYPos,
                    panelWidth * intScale, 75 * intScale, 20 * intScale, "Player Name", &playerNameInput, panelWidth / 5 * intScale, 30 * intScale, 30);
    
                elementYPos += 100 * intScale;

                if (guiContext.createButton(scaledPanelPaddingX + panelWidth / 2.0f * intScale, elementYPos,
                        panelWidth / 2.0f * intScale, 75 * intScale, buttonTextSize, ">", buttonStyle).isClicked())
                {
                    newGamePage++;
                    deferHoverRectReset = true;
                }
            }
            else if (newGamePage == 1)
            {
                guiContext.createTextEnter(scaledPanelPaddingX, elementYPos,
                    panelWidth * intScale, 75 * intScale, 20 * intScale, "World Seed", &worldSeedInput, panelWidth / 5 * intScale, 30 * intScale, 30);
                
                elementYPos += 130 * intScale;

                guiContext.createColorWheel(scaledPanelPaddingX + panelWidth * intScale / 4, elementYPos, 50, selectedBodyColorValueHSV / 100.0f, selectedBodyColor);

                guiContext.createColorWheel(scaledPanelPaddingX + panelWidth * intScale / 4 * 3, elementYPos, 50, selectedSkinColorValueHSV / 100.0f, selectedSkinColor);

                ResolutionHandler::overrideZoom(0);

                // Draw player preview
                NetworkPlayer playerPreview(pl::Vector2f(0, 0));
                playerPreview.setPosition(pl::Vector2f(scaledPanelPaddingX + panelWidth / 2 * intScale, elementYPos + (24 * intScale)), 0);
                playerPreview.setBodyColor(selectedBodyColor);
                playerPreview.setSkinColor(selectedSkinColor);

                playerPreview.draw(window, spriteBatch, game, nullptr, 0.0f, 0.0f, 1, pl::Color(), false);
                spriteBatch.endDrawing(window);

                elementYPos += 60 * intScale;

                if (guiContext.createSlider(scaledPanelPaddingX, elementYPos, panelWidth * intScale / 2, 75 * intScale,
                    0.0f, 100.0f, &selectedBodyColorValueHSV, 20 * intScale, "", panelWidth / 8 * intScale, panelWidth / 8 * intScale, 40 * intScale,
                    whiteBlackGradientSliderStyle).isHeld())
                {
                    pl::Color hsvColor = Helper::convertRGBtoHSV(selectedBodyColor);
                    selectedBodyColor = Helper::convertHSVtoRGB(hsvColor.r, hsvColor.g, selectedBodyColorValueHSV / 100.0f);
                }

                if (guiContext.createSlider(scaledPanelPaddingX + panelWidth / 2 * intScale, elementYPos, panelWidth * intScale / 2, 75 * intScale,
                    0.0f, 100.0f, &selectedSkinColorValueHSV, 20 * intScale, "", panelWidth / 8 * intScale, panelWidth / 8 * intScale, 40 * intScale,
                    whiteBlackGradientSliderStyle).isHeld())
                {
                    pl::Color hsvColor = Helper::convertRGBtoHSV(selectedSkinColor);
                    selectedSkinColor = Helper::convertHSVtoRGB(hsvColor.r, hsvColor.g, selectedSkinColorValueHSV / 100.0f);
                }
                
                elementYPos += 100 * intScale;

                if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth / 2.0f * intScale, 75 * intScale, buttonTextSize, "<", buttonStyle).isClicked())
                {
                    newGamePage--;
                    deferHoverRectReset = true;
                }
            }

            elementYPos += 200 * intScale;

            if (resolution.y < 900)
            {
                elementYPos -= 110 * intScale;
            }

            if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, buttonTextSize, "Start", buttonStyle)
                .isClicked())
            {
                if (!saveNameInput.empty() && canInteract)
                {
                    // Check save name does not already exist
                    bool nameExists = false;
                    for (const auto& saveFile : saveFileSummaries)
                    {
                        if (saveFile.name == saveNameInput)
                        {
                            nameExists = true;
                            break;
                        }
                    }
                    if (!nameExists)
                    {
                        menuEvent = MainMenuEvent();
                        menuEvent->type = MainMenuEventType::StartNew;
                        menuEvent->saveFileSummary.name = saveNameInput;
                        menuEvent->saveFileSummary.playerName = playerNameInput;
                        menuEvent->saveFileSummary.playerData.bodyColor = selectedBodyColor;
                        menuEvent->saveFileSummary.playerData.skinColor = selectedSkinColor;
                        menuEvent->worldSeed = getWorldSeedFromString(worldSeedInput);
                    }
                }
            }

            elementYPos += 100 * intScale;
            
            if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, buttonTextSize, "Back", buttonStyle)
                .isClicked())
            {
                if (canInteract)
                {
                    nextUIState = MainMenuState::Main;
                }
            }
            break;
        }
        case MainMenuState::SelectingLoad:
        {
            static constexpr int saveFilesPerPage = 4;

            ResolutionHandler::overrideZoom(0);

            for (int i = saveFilesPerPage * saveFilePage; i < std::min(static_cast<int>(saveFileSummaries.size()), saveFilesPerPage * (saveFilePage + 1)); i++)
            {
                const SaveFileSummary& saveFileSummary = saveFileSummaries[i];

                std::string saveSummaryString = saveFileSummary.name + " - (" + saveFileSummary.timePlayedString + ")";

                if (guiContext.createButton(scaledPanelPaddingX, elementYPos,
                    panelWidth * intScale, 75 * intScale, buttonTextSize, saveSummaryString, buttonStyle)
                        .isClicked())
                {
                    menuEvent = MainMenuEvent();
                    menuEvent->type = MainMenuEventType::Load;
                    menuEvent->saveFileSummary = saveFileSummary;
                }

                // Draw player preview
                NetworkPlayer playerPreview(pl::Vector2f(0, 0));
                playerPreview.setPlayerData(saveFileSummary.playerData);
                playerPreview.setPosition(pl::Vector2f(scaledPanelPaddingX + 35 * intScale, elementYPos + (60 * intScale)), 0);
                playerPreview.setArmourFromInventory(saveFileSummary.playerData.armourInventory);

                playerPreview.draw(window, spriteBatch, game, nullptr, 0.0f, 0.0f, 1, pl::Color(), false);
                spriteBatch.endDrawing(window);

                elementYPos += 100 * intScale;
            }

            // Test deletion of save
            if (guiContext.getInputState().rightMouseJustDown)
            {
                deleteSaveHoldTime = 0.0f;
                if (const GUIElement* hoveredElement = guiContext.getHoveredElement();
                    hoveredElement != nullptr)
                {
                    deletingSaveIndex = hoveredElement->getElementID() + saveFilesPerPage * saveFilePage;
                }
            }
            else if (guiContext.getInputState().rightMouseJustUp && deletingSaveIndex >= 0)
            {
                deletingSaveIndex = -1;
            }

            if (deletingSaveIndex >= 0)
            {
                const GUIElement* deletingSaveElement = guiContext.getElementByID(deletingSaveIndex % saveFilesPerPage);
                if (deletingSaveElement)
                {
                    deleteSaveHoldTime += dt;
                    deletingRect = deletingSaveElement->getBoundingBox();
                    deletingRect.width *= deleteSaveHoldTime / DELETE_SAVE_MAX_HOLD_TIME;
                }
                else
                {
                    deletingSaveIndex = -1;
                }

                if (deleteSaveHoldTime >= DELETE_SAVE_MAX_HOLD_TIME)
                {
                    GameSaveIO io(saveFileSummaries[deletingSaveIndex].name);
                    io.attemptDeleteSave();
                    saveFileSummaries = io.getSaveFiles();
                    deletingSaveIndex = -1;
                    deleteSaveHoldTime = 0.0f;
                }
            }

            // Text if no save files
            if (saveFileSummaries.size() <= 0)
            {
                pl::TextDrawData textDrawData;
                textDrawData.text = "No save files found";
                textDrawData.position = pl::Vector2f((scaledPanelPaddingX + panelWidth / 2) * intScale, elementYPos);
                textDrawData.size = 24 * intScale;
                textDrawData.centeredX = true;
                textDrawData.centeredY = true;
                textDrawData.color = pl::Color(255, 255, 255);

                TextDraw::drawText(window, textDrawData);
            }

            elementYPos = startElementYPos + (100 * intScale) * saveFilesPerPage;

            // Create page scroll buttons if required
            if (saveFileSummaries.size() > saveFilesPerPage)
            {
                // Create page back button
                if (saveFilePage > 0)
                {
                    if (guiContext.createButton(scaledPanelPaddingX, elementYPos,
                        panelWidth / 2 * intScale, 50 * intScale, buttonTextSize, "<", buttonStyle)
                            .isClicked())
                    {
                        saveFilePage--;
                        deferHoverRectReset = true; // ui may change
                        deletingSaveIndex = -1;
                    }
                }

                // Create page forward button
                if (saveFilePage < std::ceil(saveFileSummaries.size() / saveFilesPerPage))
                {
                    if (guiContext.createButton((scaledPanelPaddingX + panelWidth / 2) * intScale, elementYPos,
                        panelWidth / 2 * intScale, 50 * intScale, buttonTextSize, ">", buttonStyle)
                            .isClicked())
                    {
                        saveFilePage++;
                        deferHoverRectReset = true; // ui may change
                        deletingSaveIndex = -1;
                    }
                }

                elementYPos += 100 * intScale;
            }

            if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, buttonTextSize, "Back", buttonStyle)
                .isClicked())
            {
                if (canInteract)
                {
                    nextUIState = MainMenuState::Main;
                    deletingSaveIndex = -1;
                }
            }
            break;
        }
        case MainMenuState::JoiningGame:
        {
            pl::TextDrawData textDrawData;
            textDrawData.text = "Join Multiplayer";
            textDrawData.position = pl::Vector2f((scaledPanelPaddingX + panelWidth / 2) * intScale, elementYPos);
            textDrawData.size = 24 * intScale;
            textDrawData.centeredX = true;
            textDrawData.centeredY = true;
            textDrawData.color = pl::Color(255, 255, 255);

            TextDraw::drawText(window, textDrawData);

            elementYPos += 60 * intScale;

            guiContext.createTextEnter(scaledPanelPaddingX, elementYPos,
                panelWidth * intScale, 75 * intScale, 20 * intScale, "Player Name", &playerNameInput, panelWidth / 5 * intScale, 30 * intScale, 30);

            elementYPos += 130 * intScale;

            guiContext.createColorWheel(scaledPanelPaddingX + panelWidth * intScale / 4, elementYPos, 50, selectedBodyColorValueHSV, selectedBodyColor);

            guiContext.createColorWheel(scaledPanelPaddingX + panelWidth * intScale / 4 * 3, elementYPos, 50, selectedSkinColorValueHSV, selectedSkinColor);

            ResolutionHandler::overrideZoom(0);

            // Draw player preview
            NetworkPlayer playerPreview(pl::Vector2f(0, 0));
            playerPreview.setPosition(pl::Vector2f(scaledPanelPaddingX + panelWidth / 2 * intScale, elementYPos + (24 * intScale)), 0);
            playerPreview.setBodyColor(selectedBodyColor);
            playerPreview.setSkinColor(selectedSkinColor);

            playerPreview.draw(window, spriteBatch, game, nullptr, 0.0f, 0.0f, 1, pl::Color(), false);
            spriteBatch.endDrawing(window);

            elementYPos += 60 * intScale;

            if (guiContext.createSlider(scaledPanelPaddingX, elementYPos, panelWidth * intScale / 2, 75 * intScale,
                0.0f, 1.0f, &selectedBodyColorValueHSV, 20 * intScale, "", panelWidth / 8 * intScale, panelWidth / 8 * intScale, 40 * intScale).isHeld())
            {
                pl::Color hsvColor = Helper::convertRGBtoHSV(selectedBodyColor);
                selectedBodyColor = Helper::convertHSVtoRGB(hsvColor.r, hsvColor.g, selectedBodyColorValueHSV);
            }

            if (guiContext.createSlider(scaledPanelPaddingX + panelWidth / 2 * intScale, elementYPos, panelWidth * intScale / 2, 75 * intScale,
                0.0f, 1.0f, &selectedSkinColorValueHSV, 20 * intScale, "", panelWidth / 8 * intScale, panelWidth / 8 * intScale, 40 * intScale).isHeld())
            {
                pl::Color hsvColor = Helper::convertRGBtoHSV(selectedSkinColor);
                selectedSkinColor = Helper::convertHSVtoRGB(hsvColor.r, hsvColor.g, selectedSkinColorValueHSV);
            }
            
            elementYPos += 100 * intScale;

            if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, buttonTextSize, "Join", buttonStyle)
                .isClicked())
            {
                if (!playerNameInput.empty())
                {
                    menuEvent = MainMenuEvent();
                    menuEvent->type = MainMenuEventType::JoinGame;
                    menuEvent->saveFileSummary.playerName = playerNameInput;
                    menuEvent->saveFileSummary.playerData.bodyColor = selectedBodyColor;
                    menuEvent->saveFileSummary.playerData.skinColor = selectedSkinColor;
                }
            }

            elementYPos += 100 * intScale;
            
            if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, buttonTextSize, "Back", buttonStyle)
                .isClicked())
            {
                if (canInteract)
                {
                    nextUIState = MainMenuState::Main;
                    menuEvent = MainMenuEvent();
                    menuEvent->type = MainMenuEventType::CancelJoinGame;
                }
            }
            break;   
        }
        case MainMenuState::Options:
        {
            if (createOptionsMenu(window, elementYPos))
            {
                menuEvent = MainMenuEvent();
                menuEvent->type = MainMenuEventType::SaveOptions;

                nextUIState = MainMenuState::Main;
            }
            break;
        }
    }

    updateAndDrawSelectionHoverRect(window, dt);

    if (mainMenuState == MainMenuState::SelectingLoad && deletingSaveIndex >= 0)
    {
        pl::VertexArray deleteRect;
        deleteRect.addQuad(deletingRect, pl::Color(230, 20, 20, 150), pl::Rect<float>());

        window.draw(deleteRect, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);
    }

    if (nextUIState != mainMenuState)
    {
        changeUIState<MainMenuState>(nextUIState, mainMenuState);
    }

    guiContext.draw(window);

    guiContext.endGUI();

    // Version number text
    pl::TextDrawData versionTextDrawData;
    versionTextDrawData.text = GAME_VERSION;
    versionTextDrawData.color = pl::Color(255, 255, 255);
    versionTextDrawData.size = 24 * intScale;
    versionTextDrawData.position = pl::Vector2f(10 * intScale, resolution.y - (24 + 10) * intScale);

    TextDraw::drawText(window, versionTextDrawData);

    return menuEvent;
}

void MainMenuGUI::setMainMenuJoinGame()
{
    playerNameInput = "";
    selectedBodyColor = pl::Color(158, 69, 57);
    selectedBodyColorValueHSV = 1.0f;
    selectedSkinColor = pl::Color(230, 144, 78);
    selectedSkinColorValueHSV = 1.0f;
    changeUIState(MainMenuState::JoiningGame, mainMenuState);
}

bool MainMenuGUI::createOptionsMenu(pl::RenderTarget& window, int startElementYPos)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    pl::Vector2<uint32_t> resolution = ResolutionHandler::getResolution();

    int scaledPanelPaddingX = getScaledPanelPaddingX();

    int buttonTextSize = 24 * intScale;

    if (optionsPage == 0)
    {
        float musicVolume = Sounds::getMusicVolume();
        if (guiContext.createSlider(scaledPanelPaddingX, startElementYPos, panelWidth * intScale, 75 * intScale,
            0.0f, 100.0f, &musicVolume, 20 * intScale, "Music Volume", panelWidth / 2 * intScale, panelWidth / 10 * intScale, 40 * intScale)
            .isHeld())
        {
            Sounds::setMusicVolume(musicVolume);
        }
    
        startElementYPos += 100 * intScale;
    
        float soundVolume = Sounds::getSoundVolume();
        if (guiContext.createSlider(scaledPanelPaddingX, startElementYPos, panelWidth * intScale, 75 * intScale,
            0.0f, 100.0f, &soundVolume, 20 * intScale, "Sound Volume", panelWidth / 2 * intScale, panelWidth / 10 * intScale, 40 * intScale)
            .isHeld())
        {
            Sounds::setSoundVolume(soundVolume);
        }
    
        startElementYPos += 100 * intScale;

        bool vSyncEnabled = ResolutionHandler::getVSync();
        if (guiContext.createCheckbox(scaledPanelPaddingX, startElementYPos, panelWidth * intScale, 75 * intScale, 20 * intScale, "V-Sync", &vSyncEnabled,
            (panelWidth / 2 + 80) * intScale, (panelWidth / 10 + 80) * intScale, 40 * intScale)
            .isClicked())
        {
            ResolutionHandler::setVSync(vSyncEnabled);
        }

        startElementYPos += 100 * intScale;

        if (guiContext.createButton(scaledPanelPaddingX + panelWidth / 2 * intScale, startElementYPos,
            panelWidth / 2 * intScale, 50 * intScale, buttonTextSize, ">", buttonStyle)
                .isClicked())
        {
            optionsPage++;
            resetHoverRect();
        }
    }
    else if (optionsPage == 1)
    {
        bool screenShakeEnabled = Camera::getScreenShakeEnabled();
        if (guiContext.createCheckbox(scaledPanelPaddingX, startElementYPos, panelWidth * intScale, 75 * intScale, 20 * intScale, "Screenshake", &screenShakeEnabled,
            (panelWidth / 2 + 80) * intScale, (panelWidth / 10 + 80) * intScale, 40 * intScale)
            .isClicked())
        {
            Camera::setScreenShakeEnabled(screenShakeEnabled);
        }
        
        startElementYPos += 100 * intScale;
        
        // Create button glyph switch buttons
        if (guiContext.createButton(scaledPanelPaddingX, startElementYPos,
            panelWidth / 6 * intScale, 50 * intScale, buttonTextSize, "<", buttonStyle)
                .isClicked())
        {
            int glyphType = InputManager::getGlyphType();
            InputManager::setGlyphType(glyphType - 1);
        }
    
        if (guiContext.createButton((scaledPanelPaddingX + panelWidth / 6 * 5) * intScale, startElementYPos,
            panelWidth / 6 * intScale, 50 * intScale, buttonTextSize, ">", buttonStyle)
                .isClicked())
        {
            int glyphType = InputManager::getGlyphType();
            InputManager::setGlyphType(glyphType + 1);
        }
    
        // Text showing current controller glyph type
        pl::TextDrawData glyphTypeDrawData;
        glyphTypeDrawData.text = "Controller Glyph " + std::to_string(InputManager::getGlyphType() + 1);
        glyphTypeDrawData.position = pl::Vector2f(scaledPanelPaddingX + panelWidth / 2 * intScale, startElementYPos + 50 * 0.5f * intScale);
        glyphTypeDrawData.centeredX = true;
        glyphTypeDrawData.centeredY = true;
        glyphTypeDrawData.size = 24 * intScale;
        glyphTypeDrawData.color = pl::Color(255, 255, 255);
        TextDraw::drawText(window, glyphTypeDrawData);

        startElementYPos += 100 * intScale;

        if (guiContext.createButton(scaledPanelPaddingX, startElementYPos,
            panelWidth / 2 * intScale, 50 * intScale, buttonTextSize, "<", buttonStyle)
                .isClicked())
        {
            optionsPage--;
            resetHoverRect();
        }
    }
    
    startElementYPos = std::min(startElementYPos + 200 * intScale, resolution.y - 100 * intScale);

    if (guiContext.createButton(scaledPanelPaddingX, startElementYPos, panelWidth * intScale, 75 * intScale, buttonTextSize, "Back", buttonStyle)
        .isClicked())
    {
        return true;
    }

    return false;
}

std::optional<PauseMenuEventType> MainMenuGUI::createAndDrawPauseMenu(pl::RenderTarget& window, float dt, float applicationTime, bool steamInitialised,
    std::optional<uint64_t> lobbyId)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    pl::Vector2f resolution = static_cast<pl::Vector2f>(ResolutionHandler::getResolution());
    
    drawPanel(window);

    int scaledPanelPaddingX = getScaledPanelPaddingX();

    // Draw title
    pl::DrawData titleDrawData;
    titleDrawData.texture = TextureManager::getTexture(TextureType::UI);
    titleDrawData.shader = Shaders::getShader(ShaderType::Default);
    titleDrawData.scale = pl::Vector2f(3, 3) * intScale;
    titleDrawData.position = pl::Vector2f(scaledPanelPaddingX + panelWidth / 2 * intScale, std::round((140 + std::sin(applicationTime) * 20) * intScale));
    titleDrawData.centerRatio = pl::Vector2f(0.5f, 0.5f);
    titleDrawData.textureRect = pl::Rect<int>(21, 160, 212, 32);

    TextureManager::drawSubTexture(window, titleDrawData);

    const int startElementYPos = resolution.y * 0.37f;
    int elementYPos = startElementYPos;

    PauseMenuState nextUIState = pauseMenuState;

    std::optional<PauseMenuEventType> menuEvent = std::nullopt;

    switch (pauseMenuState)
    {
        case PauseMenuState::Main:
        {
            if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, 24 * intScale, "Resume", buttonStyle).isClicked())
            {
                menuEvent = PauseMenuEventType::Resume;
            }

            elementYPos += 100 * intScale;

            if (steamInitialised)
            {
                if (lobbyId.has_value())
                {
                    if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, 24 * intScale, "Invite Friends", buttonStyle)
                        .isClicked())
                    {
                        SteamFriends()->ActivateGameOverlayInviteDialog(lobbyId.value());
                        resetHoverRect();
                    }
                }
                else
                {
                    if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, 24 * intScale, "Start Multiplayer", buttonStyle)
                        .isClicked())
                    {
                        // showPauseMenuWishlist = true;
                        menuEvent = PauseMenuEventType::StartMultiplayer;
                        resetHoverRect();
                    }
                }

                elementYPos += 100 * intScale;
            }
            
            if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, 24 * intScale, "Options", buttonStyle).isClicked())
            {
                nextUIState = PauseMenuState::Options;
                optionsPage = 0;
            }

            elementYPos += 100 * intScale;

            if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, 24 * intScale, "Quit", buttonStyle).isClicked())
            {
                menuEvent = PauseMenuEventType::Quit;
            }

            break;
        }
        case PauseMenuState::Options:
        {
            if (createOptionsMenu(window, elementYPos))
            {
                menuEvent = PauseMenuEventType::SaveOptions;

                nextUIState = PauseMenuState::Main;
            }
        }
    }

    updateAndDrawSelectionHoverRect(window, dt);

    if (nextUIState != pauseMenuState)
    {
        changeUIState<PauseMenuState>(nextUIState, pauseMenuState);
    }

    guiContext.draw(window);

    guiContext.endGUI();

    return menuEvent;
}

int MainMenuGUI::getWorldSeedFromString(std::string string)
{
    // Get seed from input
    int seed = 0;
    if (string.empty())
    {
        seed = rand();
    }
    else
    {
        // Compute simple hash of seed
        seed = 0x55555555;
        for (char c : string)
        {
            seed ^= c;
            seed = seed << 5;
        }
    }

    return seed;
}

template <typename StateType>
void MainMenuGUI::changeUIState(StateType newState, StateType& currentState)
{
    currentState = newState;
    resetHoverRect();
    deferHoverRectReset = false;
}

void MainMenuGUI::setCanInteract(bool value)
{
    canInteract = value;
}