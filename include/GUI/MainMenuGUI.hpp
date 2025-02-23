#pragma once

#include <string>
#include <optional>

#include <extlib/steam/steam_api.h>
#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Camera.hpp"
#include "Core/SpriteBatch.hpp"
#include "Core/InputManager.hpp"

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

enum class PauseMenuState
{
    Main,
    Options
};

enum class PauseMenuEventType
{
    Resume,
    StartMultiplayer,
    SaveOptions,
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

    void initialisePauseMenu();

    void update(float dt, sf::Vector2f mouseScreenPos, Game& game, ProjectileManager& projectileManager);

    std::optional<MainMenuEvent> createAndDraw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, float dt, float gameTime);

    std::optional<PauseMenuEventType> createAndDrawPauseMenu(sf::RenderTarget& window, float dt, float gameTime, bool steamInitialised, std::optional<uint64_t> lobbyId);

    void setCanInteract(bool value);

private:
    int getWorldSeedFromString(std::string string);

    template <typename StateType>
    void changeUIState(StateType newState, StateType& currentState);

    // Returns true if back is pressed
    bool createOptionsMenu(sf::RenderTarget& window, int startElementYPos);

private:
    std::string menuErrorMessage;

    // Menu
    MainMenuState mainMenuState;
    PauseMenuState pauseMenuState;

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