#include "GUI/MainMenuGUI.hpp"
#include "Game.hpp"

MainMenuGUI::MainMenuGUI()
{

}

void MainMenuGUI::initialise()
{
    canInteract = true;
    mainMenuState = MainMenuState::Main;

    static const std::string backgroundWorldSeed = "Planeturem";
    backgroundChunkManager.setSeed(getWorldSeedFromString(backgroundWorldSeed));
    backgroundChunkManager.setPlanetType(0);

    int worldSize = backgroundChunkManager.getWorldSize();
    worldViewPosition.x = rand() % (worldSize * static_cast<int>(CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED));
    worldViewPosition.y = rand() % (worldSize * static_cast<int>(CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED));
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

    // Draw background chunks / world
    std::vector<WorldObject*> worldObjects = backgroundChunkManager.getChunkObjects();
    std::vector<WorldObject*> entities = backgroundChunkManager.getChunkEntities();
    worldObjects.insert(worldObjects.end(), entities.begin(), entities.end());

    sf::RenderTexture worldTexture;
    game.drawWorld(worldTexture, dt, worldObjects, backgroundChunkManager, backgroundCamera);

    sf::Sprite worldTextureSprite(worldTexture.getTexture());
    window.draw(worldTextureSprite);

    // Draw title
    TextureDrawData titleDrawData;
    titleDrawData.type = TextureType::UI;
    titleDrawData.scale = sf::Vector2f(3, 3) * intScale;
    titleDrawData.position = sf::Vector2f(std::round(window.getSize().x / 2.0f), std::round((200 + std::sin(gameTime) * 20) * intScale));
    titleDrawData.centerRatio = sf::Vector2f(0.5f, 0.5f);

    TextureManager::drawSubTexture(window, titleDrawData, sf::IntRect(21, 160, 212, 32));

    // Buttons / UI
    switch (mainMenuState)
    {
        case MainMenuState::Main:
        {
            if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, window.getSize().y / 2.0f - 200.0f * intScale, 200 * intScale, 75 * intScale, "New"))
            {
                saveNameInput = "";
                worldSeedInput = "";
                mainMenuState = MainMenuState::StartingNew;
            }

            if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, window.getSize().y / 2.0f - 50 * intScale, 200 * intScale, 75 * intScale, "Load"))
            {
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
                MainMenuEvent quitEvent;
                quitEvent.type = MainMenuEventType::Quit;
                return quitEvent;
            }
            break;
        }
        case MainMenuState::StartingNew:
        {
            guiContext.createTextEnter(window.getSize().x / 2.0f - 200 * intScale, window.getSize().y / 2.0f - 100.0f * intScale,
                400 * intScale, 40 * intScale, "Name", &saveNameInput);

            guiContext.createTextEnter(window.getSize().x / 2.0f - 200 * intScale, window.getSize().y / 2.0f + 150 * intScale,
            400 * intScale, 40 * intScale, "Seed", &worldSeedInput);

            if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, window.getSize().y / 2.0f + 300 * intScale, 200 * intScale, 75 * intScale, "Start"))
            {
                if (!saveNameInput.empty())
                {
                    MainMenuEvent startEvent;
                    startEvent.type = MainMenuEventType::StartNew;
                    startEvent.saveName = saveNameInput;
                    startEvent.worldSeed = getWorldSeedFromString(saveNameInput);
                    return startEvent;
                }
            }
            
            if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, window.getSize().y - 150 * intScale, 200 * intScale, 75 * intScale, "Back"))
            {
                if (canInteract)
                {
                    mainMenuState = MainMenuState::Main;
                }
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
                    MainMenuEvent loadEvent;
                    loadEvent.type = MainMenuEventType::Load;
                    loadEvent.saveName = saveName;
                    return loadEvent;
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
                if (canInteract)
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

void MainMenuGUI::setCanInteract(bool value)
{
    canInteract = value;
}