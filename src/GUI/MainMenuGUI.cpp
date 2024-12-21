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
    backgroundChunkManager.setSeed(rand());
    backgroundChunkManager.setPlanetType(0);

    int worldSize = backgroundChunkManager.getWorldSize();
    ChunkPosition worldViewChunk = backgroundChunkManager.findValidSpawnChunk(2);
    worldViewPosition.x = worldViewChunk.x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    worldViewPosition.y = worldViewChunk.y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;

    backgroundCamera.instantUpdate(worldViewPosition);
}

void MainMenuGUI::update(float dt, sf::Vector2f mouseScreenPos, Game& game, ProjectileManager& projectileManager, InventoryData& inventory)
{
    // Update background world / chunk manager
    worldViewPosition.x += 20.0f * dt;
    worldViewPosition.y += 30.0f * dt;

    backgroundCamera.update(worldViewPosition, sf::Vector2f(0, 0), dt);

    backgroundChunkManager.updateChunks(game, backgroundCamera);
    backgroundChunkManager.updateChunksObjects(game, dt);
    backgroundChunkManager.updateChunksEntities(dt, projectileManager, inventory);
}

std::optional<MainMenuEvent> MainMenuGUI::createAndDraw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, float dt, float gameTime)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    sf::Vector2f resolution = static_cast<sf::Vector2f>(ResolutionHandler::getResolution());
    
    // Draw background chunks / world
    std::vector<WorldObject*> worldObjects = backgroundChunkManager.getChunkObjects();
    std::vector<WorldObject*> entities = backgroundChunkManager.getChunkEntities();
    worldObjects.insert(worldObjects.end(), entities.begin(), entities.end());

    sf::RenderTexture worldTexture;
    game.drawWorld(worldTexture, dt, worldObjects, backgroundChunkManager, backgroundCamera);

    sf::Sprite worldTextureSprite(worldTexture.getTexture());
    window.draw(worldTextureSprite);

    drawPanel(window);

    int scaledPanelPaddingX = getScaledPanelPaddingX();

    // Draw title
    TextureDrawData titleDrawData;
    titleDrawData.type = TextureType::UI;
    titleDrawData.scale = sf::Vector2f(3, 3) * intScale;
    titleDrawData.position = sf::Vector2f(scaledPanelPaddingX + panelWidth / 2 * intScale, std::round((140 + std::sin(gameTime) * 20) * intScale));
    titleDrawData.centerRatio = sf::Vector2f(0.5f, 0.5f);

    TextureManager::drawSubTexture(window, titleDrawData, sf::IntRect(21, 160, 212, 32));

    MainMenuState nextUIState = mainMenuState;

    const int startElementYPos = resolution.y * 0.37f;
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
            guiContext.createTextEnter(scaledPanelPaddingX, elementYPos,
                panelWidth * intScale, 75 * intScale, 20 * intScale, "Name", &saveNameInput, panelWidth / 5 * intScale, 30 * intScale, 30);

            elementYPos += 150 * intScale;

            guiContext.createTextEnter(scaledPanelPaddingX, elementYPos,
                panelWidth * intScale, 75 * intScale, 20 * intScale, "Seed", &worldSeedInput, panelWidth / 5 * intScale, 30 * intScale, 30);

            elementYPos += 200 * intScale;

            if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, buttonTextSize, "Start", buttonStyle)
                .isClicked())
            {
                if (!saveNameInput.empty())
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
                    deletingRect = static_cast<sf::FloatRect>(deletingSaveElement->getBoundingBox());
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
                TextDrawData textDrawData;
                textDrawData.text = "No save files found";
                textDrawData.position = sf::Vector2f((scaledPanelPaddingX + panelWidth / 2) * intScale, elementYPos);
                textDrawData.size = 24 * intScale;
                textDrawData.centeredX = true;
                textDrawData.centeredY = true;
                textDrawData.colour = sf::Color(255, 255, 255);

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
        case MainMenuState::Options:
        {
            float musicVolume = Sounds::getMusicVolume();
            if (guiContext.createSlider(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale,
                0.0f, 100.0f, &musicVolume, 20 * intScale, "Music Volume", panelWidth / 2 * intScale, panelWidth / 10 * intScale, 40 * intScale)
                .isHeld())
            {
                Sounds::setMusicVolume(musicVolume);
            }

            elementYPos += 300 * intScale;

            if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, buttonTextSize, "Back", buttonStyle)
                .isClicked())
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
        sf::RectangleShape deleteRectDraw;
        deleteRectDraw.setPosition(sf::Vector2f(deletingRect.left, deletingRect.top));
        deleteRectDraw.setSize(sf::Vector2f(deletingRect.width, deletingRect.height));
        deleteRectDraw.setFillColor(sf::Color(230, 20, 20, 150));

        window.draw(deleteRectDraw);
    }

    if (nextUIState != mainMenuState)
    {
        changeUIState(nextUIState);
    }

    guiContext.draw(window);

    guiContext.endGUI();

    // Version number text
    TextDrawData versionTextDrawData;
    versionTextDrawData.text = GAME_VERSION;
    versionTextDrawData.colour = sf::Color(255, 255, 255);
    versionTextDrawData.size = 24 * intScale;
    versionTextDrawData.position = sf::Vector2f(10 * intScale, resolution.y - (24 + 10 ) * intScale);

    TextDraw::drawText(window, versionTextDrawData);

    return menuEvent;
}

std::optional<PauseMenuEventType> MainMenuGUI::createAndDrawPauseMenu(sf::RenderTarget& window, float dt, float gameTime)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    sf::Vector2f resolution = static_cast<sf::Vector2f>(ResolutionHandler::getResolution());
    
    drawPanel(window);

    int scaledPanelPaddingX = getScaledPanelPaddingX();

    // Draw title
    TextureDrawData titleDrawData;
    titleDrawData.type = TextureType::UI;
    titleDrawData.scale = sf::Vector2f(3, 3) * intScale;
    titleDrawData.position = sf::Vector2f((scaledPanelPaddingX + panelWidth / 2 * intScale), std::round((140 + std::sin(gameTime) * 20) * intScale));
    titleDrawData.centerRatio = sf::Vector2f(0.5f, 0.5f);

    TextureManager::drawSubTexture(window, titleDrawData, sf::IntRect(21, 160, 212, 32));

    const int startElementYPos = resolution.y * 0.37f;
    int elementYPos = startElementYPos;

    std::optional<PauseMenuEventType> menuEvent = std::nullopt;

    if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, 24 * intScale, "Resume", buttonStyle).isClicked())
    {
        menuEvent = PauseMenuEventType::Resume;
    }

    elementYPos += 100 * intScale;

    if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, 24 * intScale, "Quit", buttonStyle).isClicked())
    {
        menuEvent = PauseMenuEventType::Quit;
    }

    updateAndDrawSelectionHoverRect(window, dt);

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

void MainMenuGUI::changeUIState(MainMenuState newState)
{
    mainMenuState = newState;
    resetHoverRect();
    deferHoverRectReset = false;
}

void MainMenuGUI::setCanInteract(bool value)
{
    canInteract = value;
}