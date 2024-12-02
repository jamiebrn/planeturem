#pragma once

#include <string>
#include <optional>

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Camera.hpp"
#include "Core/SpriteBatch.hpp"

#include "IO/GameSaveIO.hpp"

#include "World/ChunkManager.hpp"

#include "GUI/Base/GUIContext.hpp"
#include "GUI/DefaultGUIPanel.hpp"

enum class MainMenuState
{
    Main,
    StartingNew,
    SelectingLoad,
    Options
};

enum class MainMenuEventType
{
    StartNew,
    Load,
    SaveOptions,
    Quit
};

struct MainMenuEvent
{
    MainMenuEventType type;
    SaveFileSummary saveFileSummary;
    int worldSeed;
};

enum class PauseMenuEventType
{
    Resume,
    Quit
};

class Game;
class ProjectileManager;
class InventoryData;

class MainMenuGUI : public DefaultGUIPanel
{
public:
    MainMenuGUI() = default;

    void initialise();

    void update(float dt, sf::Vector2f mouseScreenPos, Game& game, ProjectileManager& projectileManager, InventoryData& inventory);

    std::optional<MainMenuEvent> createAndDraw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, float dt, float gameTime);

    std::optional<PauseMenuEventType> createAndDrawPauseMenu(sf::RenderTarget& window, float dt, float gameTime);

    void setCanInteract(bool value);

private:
    int getWorldSeedFromString(std::string string);
    
    void changeUIState(MainMenuState newState);

private:
    std::string menuErrorMessage;

    // Menu
    MainMenuState mainMenuState;

    std::vector<SaveFileSummary> saveFileSummaries;
    int saveFilePage;

    std::string saveNameInput;
    std::string worldSeedInput;

    static constexpr float DELETE_SAVE_MAX_HOLD_TIME = 3.0f;
    float deleteSaveHoldTime;
    int deletingSaveIndex;
    sf::FloatRect deletingRect;

    bool canInteract;

    ChunkManager backgroundChunkManager;
    sf::Vector2f worldViewPosition;
    Camera backgroundCamera;

};