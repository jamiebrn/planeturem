#pragma once

#include <string>
#include <optional>

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Camera.hpp"
#include "Core/SpriteBatch.hpp"

#include "IO/GameSaveIO.hpp"

#include "World/ChunkManager.hpp"

#include "Types/GameState.hpp"
#include "GUI/Base/GUIContext.hpp"

enum class MainMenuEventType
{
    StartNew,
    Load,
    Quit
};

struct MainMenuEvent
{
    MainMenuEventType type;
    std::string saveName;
    int worldSeed;
};

class Game;
class ProjectileManager;
class InventoryData;

class MainMenuGUI
{
public:
    MainMenuGUI();

    void initialise();

    void handleEvent(sf::Event& event);

    void update(float dt, sf::Vector2f mouseScreenPos, Game& game, ProjectileManager& projectileManager, InventoryData& inventory);

    std::optional<MainMenuEvent> createAndDraw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, float dt, float gameTime);

    void setCanInteract(bool value);

private:
    int getWorldSeedFromString(std::string string);

private:
    std::string menuErrorMessage;

    // Menu
    MainMenuState mainMenuState;
    // int menuScreenshotIndex;
    // float menuScreenshotTimer;

    std::vector<std::string> saveFileNames;
    int saveFilePage;

    std::string saveNameInput;
    std::string worldSeedInput;

    bool canInteract;

    ChunkManager backgroundChunkManager;
    sf::Vector2f worldViewPosition;
    Camera backgroundCamera;

    GUIContext guiContext;

};