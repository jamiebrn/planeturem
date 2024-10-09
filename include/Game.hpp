#pragma once

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <extlib/steam/steam_api.h>

#include <World/FastNoise.h>
#include <Core/json.hpp>
#include <cmath>
#include <array>
#include <unordered_map>
#include <iostream>
#include <string>
#include <memory>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/Sounds.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Helper.hpp"
#include "Core/Tween.hpp"
#include "Core/SpriteBatch.hpp"

#include "World/ChunkManager.hpp"
#include "World/ChestDataPool.hpp"
#include "World/TileMap.hpp"

#include "World/Room.hpp"
#include "World/RoomPool.hpp"

#include "Player/Player.hpp"
#include "Player/Cursor.hpp"
#include "Player/InventoryData.hpp"

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/ChestObject.hpp"
#include "Object/RocketObject.hpp"
#include "Object/StructureObject.hpp"
#include "Object/ParticleSystem.hpp"

#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ItemData.hpp"
#include "Data/RecipeData.hpp"
#include "Data/EntityData.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/RecipeDataLoader.hpp"
#include "Data/EntityDataLoader.hpp"
#include "Data/ToolDataLoader.hpp"
#include "Data/PlanetGenDataLoader.hpp"
#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"

#include "Types/GameState.hpp"
#include "Types/WorldMenuState.hpp"

#include "GUI/Base/GUIContext.hpp"
#include "GUI/InventoryGUI.hpp"
#include "GUI/TravelSelectGUI.hpp"

#include "IO/GameSaveIO.hpp"

#include "DebugOptions.hpp"

class Game
{
public:
    Game();

    bool initialise();

    void run();

private:

    // -- Main Menu -- //

    void runMainMenu(float dt);


    // -- On Planet -- //

    void runOnPlanet(float dt);

    void updateOnPlanet(float dt);
    void drawOnPlanet(float dt);

    void drawWorld(float dt, std::vector<WorldObject*>& worldObjects, std::vector<WorldObject*>& entities);
    void drawLighting(float dt, std::vector<WorldObject*>& worldObjects, std::vector<WorldObject*>& entities);

    void updateDayNightCycle(float dt);

    void testEnterStructure();
    void testExitStructure();

    // Rocket
    void enterRocket(ObjectReference rocketObjectReference, RocketObject& rocket);
    void exitRocket();
    void startFlyingRocket(PlanetType destination, bool flyingDownwards = false);
    void updateFlyingRocket(float dt, bool flyingDownwards = false);
    std::vector<PlanetType> getRocketAvailableDestinations();


    // -- In Structure -- //

    void updateInStructure(float dt);
    void drawInStructure(float dt);


    // -- Tools / interactions -- //

    void changePlayerTool();
    void attemptUseTool();
    void attemptUseToolPickaxe();
    void attemptUseToolFishingRod();

    void catchRandomFish(sf::Vector2i fishedTile);

    void attemptObjectInteract();
    void attemptBuildObject();
    void attemptPlaceLand();

    void drawGhostPlaceObjectAtCursor(ObjectType object);
    void drawGhostPlaceLandAtCursor();

    // Returns pointer (may be null) to selected / hovered object
    // Depending on game state, will check chunks / room
    BuildableObject* getSelectedObjectFromChunkOrRoom();

    // For chunks, will use chunk and tile from object reference
    // For rooms, will use tile from object reference
    BuildableObject* getObjectFromChunkOrRoom(ObjectReference objectReference);


    // -- Inventory / Chests -- //

    void giveStartingInventory();

    void initChestInData(ChestObject& chest);
    void removeChestFromData(ChestObject& chest);
    void checkChestOpenInRange();
    void handleOpenChestPositionWorldWrap(sf::Vector2f positionDelta);
    void closeChest();


    // -- Planet travelling -- //

    void travelToPlanet(PlanetType planetType);
    void initialiseNewPlanet(PlanetType planetType, bool placeRocket = false);

    void resetChestDataPool();
    void resetStructureRoomPool();


    // -- Game State / Transition -- //

    void updateStateTransition(float dt);
    void drawStateTransition();
    bool isStateTransitioning();
    void startChangeStateTransition(GameState newState);
    void changeState(GameState newState);

    void setWorldSeedFromInput();


    // -- Save / load -- //

    void startNewGame();
    bool saveGame(bool gettingInRocket = false);
    bool loadGame(const std::string& saveName);
    bool loadPlanet(PlanetType planetType);


    // -- Window -- //

    void handleZoom(int zoomChange);

    void handleEventsWindow(sf::Event& event);

    void toggleFullScreen();
    void handleWindowResize(sf::Vector2u newSize);


    // -- Misc -- //

    void generateWaterNoiseTexture();

    void updateMusic(float dt);

    void drawMouseCursor();

    void drawDebugMenu(float dt);


private:
    sf::RenderWindow window;
    sf::View view;
    sf::Image icon;
    bool fullScreen = true;

    GUIContext guiContext;
    std::string worldSeed;
    std::string currentSaveName;
    std::string menuErrorMessage;
    
    // Menu
    MainMenuState mainMenuState;
    int menuScreenshotIndex;
    float menuScreenshotTimer;

    std::vector<std::string> saveFileNames;
    int saveFilePage;

    SpriteBatch spriteBatch;
    sf::RenderTexture worldTexture;

    bool steamInitialised;

    sf::Clock clock;
    float gameTime;

    float dayNightToggleTimer;
    float worldDarkness;
    bool isDay;

    static constexpr float MUSIC_GAP_MIN = 5.0f;
    std::optional<MusicType> musicTypePlaying;
    float musicGapTimer;
    float musicGap;

    Player player;
    InventoryData inventory;

    ChunkManager chunkManager;

    ParticleSystem particleSystem;

    std::array<sf::Texture, 2> waterNoiseTextures;

    GameState gameState;
    GameState destinationGameState;
    static constexpr float TRANSITION_STATE_FADE_TIME = 0.3f;
    float transitionGameStateTimer;

    WorldMenuState worldMenuState;

    std::unordered_map<std::string, int> nearbyCraftingStationLevels;

    // 0xFFFF chest ID reserved for no chest opened / non-initialised chest
    uint16_t openedChestID;
    ObjectReference openedChest;
    sf::Vector2f openedChestPos;
    ChestDataPool chestDataPool;

    // Structure
    uint32_t structureEnteredID;
    sf::Vector2f structureEnteredPos;
    RoomPool structureRoomPool;

    // Rocket
    ObjectReference rocketObject;
    PlanetType destinationPlanet;

    Tween<float> floatTween;
};