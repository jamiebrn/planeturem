#include "GUI/MainMenuGUI.hpp"
#include "Game.hpp"

MainMenuGUI::MainMenuGUI()
{

}

void MainMenuGUI::initialise()
{
    canInteract = true;
    mainMenuState = MainMenuState::Main;

    deferHoverRectReset = false;

    // selectionHoverRectDrawing = false;

    // static const std::string backgroundWorldSeed = "Planeturem";
    backgroundChunkManager.setSeed(rand());
    backgroundChunkManager.setPlanetType(0);

    int worldSize = backgroundChunkManager.getWorldSize();
    ChunkPosition worldViewChunk = backgroundChunkManager.findValidSpawnChunk(2);
    worldViewPosition.x = worldViewChunk.x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    worldViewPosition.y = worldViewChunk.y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
}

void MainMenuGUI::handleEvent(sf::Event& event)
{
    guiContext.processEvent(event);
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
    // Drawing
    window.clear();

    float intScale = ResolutionHandler::getResolutionIntegerScale();
    sf::Vector2f resolution = static_cast<sf::Vector2f>(ResolutionHandler::getResolution());

    const int panelPaddingX = 250 * resolution.x / 1920.0f;
    const int panelWidth = 500;
    
    // Draw background chunks / world
    std::vector<WorldObject*> worldObjects = backgroundChunkManager.getChunkObjects();
    std::vector<WorldObject*> entities = backgroundChunkManager.getChunkEntities();
    worldObjects.insert(worldObjects.end(), entities.begin(), entities.end());

    sf::RenderTexture worldTexture;
    game.drawWorld(worldTexture, dt, worldObjects, backgroundChunkManager, backgroundCamera);

    sf::Sprite worldTextureSprite(worldTexture.getTexture());
    window.draw(worldTextureSprite);

    // Draw panel

    sf::VertexArray panel(sf::Quads);
    panel.append(sf::Vertex(sf::Vector2f((panelPaddingX) * intScale, 0), sf::Color(30, 30, 30, 180)));
    panel.append(sf::Vertex(sf::Vector2f((panelPaddingX + panelWidth) * intScale, 0), sf::Color(30, 30, 30, 180)));
    panel.append(sf::Vertex(sf::Vector2f((panelPaddingX + panelWidth) * intScale, resolution.y), sf::Color(30, 30, 30, 180)));
    panel.append(sf::Vertex(sf::Vector2f((panelPaddingX) * intScale, resolution.y), sf::Color(30, 30, 30, 180)));

    window.draw(panel);

    // Draw title
    TextureDrawData titleDrawData;
    titleDrawData.type = TextureType::UI;
    titleDrawData.scale = sf::Vector2f(3, 3) * intScale;
    titleDrawData.position = sf::Vector2f((panelPaddingX + panelWidth / 2) * intScale, std::round((140 + std::sin(gameTime) * 20) * intScale));
    titleDrawData.centerRatio = sf::Vector2f(0.5f, 0.5f);

    TextureManager::drawSubTexture(window, titleDrawData, sf::IntRect(21, 160, 212, 32));

    const ButtonStyle buttonStyle = {
        .colour = sf::Color(0, 0, 0, 0),
        .hoveredColour = sf::Color(0, 0, 0, 0),
        .clickedColour = sf::Color(0, 0, 0, 0),
        .textColour = sf::Color(200, 200, 200),
        .hoveredTextColour = sf::Color(50, 50, 50),
        .clickedTextColour = sf::Color(255, 255, 255)
    };

    MainMenuState nextUIState = mainMenuState;

    const int startElementYPos = resolution.y * 0.37f;
    int elementYPos = startElementYPos;

    // Buttons / UI
    switch (mainMenuState)
    {
        case MainMenuState::Main:
        {
            if (const Button& button = guiContext.createButton(
                panelPaddingX * intScale, elementYPos * intScale, panelWidth * intScale, 75 * intScale, "New", buttonStyle);
                button.isClicked())
            {
                saveNameInput = "";
                worldSeedInput = "";
                nextUIState = MainMenuState::StartingNew;
            }

            elementYPos += 100;

            if (const Button& button = guiContext.createButton(
                panelPaddingX * intScale, elementYPos * intScale, panelWidth * intScale, 75 * intScale, "Load", buttonStyle);
                button.isClicked())
            {
                nextUIState = MainMenuState::SelectingLoad;
                
                GameSaveIO io;
                saveFileSummaries = io.getSaveFiles();

                saveFilePage = 0;
            }

            elementYPos += 100;

            if (const Button& button = guiContext.createButton(
                panelPaddingX * intScale, elementYPos * intScale, panelWidth * intScale, 75 * intScale, "Options", buttonStyle);
                button.isClicked())
            {
                nextUIState = MainMenuState::Options;
            }

            elementYPos += 100;

            if (const Button& button = guiContext.createButton(
                panelPaddingX * intScale, elementYPos * intScale, panelWidth * intScale, 75 * intScale, "Exit", buttonStyle);
                button.isClicked())
            {
                MainMenuEvent quitEvent;
                quitEvent.type = MainMenuEventType::Quit;
                return quitEvent;
            }
            break;
        }
        case MainMenuState::StartingNew:
        {
            guiContext.createTextEnter(panelPaddingX * intScale, elementYPos * intScale,
                panelWidth * intScale, 75 * intScale, "Name", &saveNameInput, panelWidth / 5 * intScale, 30 * intScale, 30);

            elementYPos += 150;

            guiContext.createTextEnter(panelPaddingX * intScale, elementYPos * intScale,
                panelWidth * intScale, 75 * intScale, "Seed", &worldSeedInput, panelWidth / 5 * intScale, 30 * intScale, 30);

            elementYPos += 200;

            if (guiContext.createButton(panelPaddingX * intScale, elementYPos * intScale, panelWidth * intScale, 75 * intScale, "Start", buttonStyle)
                .isClicked())
            {
                if (!saveNameInput.empty())
                {
                    MainMenuEvent startEvent;
                    startEvent.type = MainMenuEventType::StartNew;
                    startEvent.saveFileSummary.name = saveNameInput;
                    startEvent.worldSeed = getWorldSeedFromString(worldSeedInput);
                    return startEvent;
                }
            }

            elementYPos += 100;
            
            if (guiContext.createButton(panelPaddingX * intScale, elementYPos * intScale, panelWidth * intScale, 75 * intScale, "Back", buttonStyle)
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

                if (guiContext.createButton(panelPaddingX * intScale, elementYPos * intScale,
                    panelWidth * intScale, 75 * intScale, saveSummaryString, buttonStyle)
                        .isClicked())
                {
                    MainMenuEvent loadEvent;
                    loadEvent.type = MainMenuEventType::Load;
                    loadEvent.saveFileSummary = saveFileSummary;
                    return loadEvent;
                }

                elementYPos += 100;
            }

            // Text if no save files
            if (saveFileSummaries.size() <= 0)
            {
                TextDrawData textDrawData;
                textDrawData.text = "No save files found";
                textDrawData.position = sf::Vector2f(panelPaddingX * intScale, elementYPos * intScale);
                textDrawData.size = 24 * intScale;
                textDrawData.centeredX = true;
                textDrawData.centeredY = true;
                textDrawData.colour = sf::Color(255, 255, 255);

                TextDraw::drawText(window, textDrawData);
            }

            elementYPos = startElementYPos + (100 * intScale) * saveFilesPerPage;

            // Create page scroll buttons if 
            if (saveFileSummaries.size() > saveFilesPerPage)
            {
                // Create page back button
                if (saveFilePage > 0)
                {
                    if (guiContext.createButton(panelPaddingX * intScale, elementYPos * intScale,
                        panelWidth / 2 * intScale, 50 * intScale, "<", buttonStyle)
                            .isClicked())
                    {
                        saveFilePage--;
                        deferHoverRectReset = true; // ui may change
                    }
                }

                // Create page forward button
                if (saveFilePage < std::ceil(saveFileSummaries.size() / saveFilesPerPage))
                {
                    if (guiContext.createButton((panelPaddingX + panelWidth / 2) * intScale, elementYPos * intScale,
                        panelWidth / 2 * intScale, 50 * intScale, ">", buttonStyle)
                            .isClicked())
                    {
                        saveFilePage++;
                        deferHoverRectReset = true; // ui may change
                    }
                }

                elementYPos += 100;
            }

            if (guiContext.createButton(panelPaddingX * intScale, elementYPos * intScale, panelWidth * intScale, 75 * intScale, "Back", buttonStyle)
                .isClicked())
            {
                if (canInteract)
                {
                    nextUIState = MainMenuState::Main;
                }
            }
            break;
        }
        case MainMenuState::Options:
        {
            float musicVolume = Sounds::getMusicVolume();
            if (guiContext.createSlider(panelPaddingX * intScale, elementYPos, panelWidth * intScale, 75 * intScale,
                0.0f, 100.0f, &musicVolume, "Music Volume", panelWidth / 2 * intScale, panelWidth / 10 * intScale, 40 * intScale)
                .isHeld())
            {
                Sounds::setMusicVolume(musicVolume);
            }

            elementYPos += 300;

            if (guiContext.createButton(panelPaddingX * intScale, elementYPos * intScale, panelWidth * intScale, 75 * intScale, "Back", buttonStyle)
                .isClicked())
            {
                nextUIState = MainMenuState::Main;
            }
            break;
        }
    }

    if (const GUIElement* hoveredElement = guiContext.getHoveredElement();
        hoveredElement != nullptr)
    {
        updateSelectionHoverRect(hoveredElement->getBoundingBox());
    }

    CollisionRect panelCollisionRect(panelPaddingX * intScale, 0, panelWidth * intScale, resolution.y);

    if (deferHoverRectReset || !panelCollisionRect.isPointInRect(guiContext.getInputState().mouseX, guiContext.getInputState().mouseY))
    {
        resetHoverRect();
    }

    if (nextUIState != mainMenuState)
    {
        changeUIState(nextUIState);
    }

    // Selection hover rect
    // Lerp towards destination
    selectionHoverRect.left = Helper::lerp(selectionHoverRect.left, selectionHoverRectDestination.left, 15.0f * dt);
    selectionHoverRect.top = Helper::lerp(selectionHoverRect.top, selectionHoverRectDestination.top, 15.0f * dt);
    selectionHoverRect.width = Helper::lerp(selectionHoverRect.width, selectionHoverRectDestination.width, 15.0f * dt);
    selectionHoverRect.height = Helper::lerp(selectionHoverRect.height, selectionHoverRectDestination.height, 15.0f * dt);

    // Draw
    sf::RectangleShape selectionRect;
    selectionRect.setPosition(sf::Vector2f(selectionHoverRect.left, selectionHoverRect.top));
    selectionRect.setSize(sf::Vector2f(selectionHoverRect.width, selectionHoverRect.height));
    selectionRect.setFillColor(sf::Color(200, 200, 200, 150));

    window.draw(selectionRect);

    guiContext.draw(window);

    guiContext.endGUI();

    return std::nullopt;
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

void MainMenuGUI::updateSelectionHoverRect(sf::IntRect destinationRect)
{
    selectionHoverRectDestination = static_cast<sf::FloatRect>(destinationRect);

    // If hover rect is 0, 0, 0, 0 (i.e. null), do not lerp, immediately set to destination
    if (selectionHoverRect == sf::FloatRect(0, 0, 0, 0))
    {
        selectionHoverRect = selectionHoverRectDestination;
    }
}

void MainMenuGUI::changeUIState(MainMenuState newState)
{
    mainMenuState = newState;
    resetHoverRect();
}

void MainMenuGUI::resetHoverRect()
{
    selectionHoverRectDestination = sf::FloatRect(0, 0, 0, 0);
    selectionHoverRect = selectionHoverRectDestination;

    deferHoverRectReset = false;
}

void MainMenuGUI::setCanInteract(bool value)
{
    canInteract = value;
}