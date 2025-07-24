#pragma once

#include <string>
#include <optional>

#include <extlib/steam/steam_api.h>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/TextDraw.hpp"
#include "Core/Camera.hpp"
#include "Core/InputManager.hpp"

#include "IO/GameSaveIO.hpp"

#include "World/WorldData.hpp"
#include "World/ChunkManager.hpp"

#include "Player/NetworkPlayer.hpp"

#include "GUI/Base/GUIContext.hpp"
#include "GUI/DefaultGUIPanel.hpp"

enum class MainMenuState
{
    Main,
    StartingNew,
    SelectingLoad,
    JoiningGame,
    Options
};

enum class MainMenuEventType
{
    StartNew,
    Load,
    JoinGame,
    CancelJoinGame,
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

    void update(float dt, pl::Vector2f mouseScreenPos, Game& game);

    std::optional<MainMenuEvent> createAndDraw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, float dt, float applicationTime);

    void setMainMenuJoinGame();

    std::optional<PauseMenuEventType> createAndDrawPauseMenu(pl::RenderTarget& window, float dt, float applicationTime, bool steamInitialised, std::optional<uint64_t> lobbyId);

    void setCanInteract(bool value);

    void setErrorMessage(const std::string& errorMessage);

private:
    int getWorldSeedFromString(std::string string);

    template <typename StateType>
    void changeUIState(StateType newState, StateType& currentState);

    // Returns true if back is pressed
    bool createOptionsMenu(pl::RenderTarget& window, int startElementYPos);

    void resetTitleYPosition();

private:
    std::string menuErrorMessage;

    // Menu
    MainMenuState mainMenuState;
    PauseMenuState pauseMenuState;
    
    static constexpr float TITLE_Y_POSITION_DEFAULT = 140;
    float titleYPosition;
    float titleYPositionDest;

    std::vector<SaveFileSummary> saveFileSummaries;
    int saveFilePage;

    std::string saveNameInput;
    std::string playerNameInput;
    std::string worldSeedInput;
    int newGamePage;

    pl::Color selectedBodyColor;
    float selectedBodyColorValueHSV = 100.0f;
    pl::Color selectedSkinColor;
    float selectedSkinColorValueHSV = 100.0f;

    static constexpr float DELETE_SAVE_MAX_HOLD_TIME = 3.0f;
    float deleteSaveHoldTime;
    int deletingSaveIndex;
    pl::Rect<float> deletingRect;

    int optionsPage;

    bool canInteract;

    WorldData menuWorldData;
    pl::Vector2f worldViewPosition;
    Camera menuCamera;

    // Error message
    static constexpr float MAX_ERROR_MESSAGE_TIME = 5.0f;
    float errorMessageTime = 0.0f;
    std::string errorMessage;

    // Demo stuff
    // bool showPauseMenuWishlist;

};