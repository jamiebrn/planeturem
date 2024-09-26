#pragma once

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <steam_api.h>

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
// #include "Player/FurnaceJob.hpp"

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/StructureObject.hpp"

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

#include "GUI/InventoryGUI.hpp"
#include "GUI/FurnaceGUI.hpp"

#include "DebugOptions.hpp"

class Game
{
public:
    Game();

    bool initialise();

    void run();

private:
    void runOnPlanet(float dt);

    void updateOnPlanet(float dt);
    void drawOnPlanet(float dt);

    void updateInStructure(float dt);
    void drawInStructure(float dt);

    void changePlayerTool();
    void attemptUseTool();
    void attemptUseToolPickaxe();
    void attemptUseToolFishingRod();

    void attemptObjectInteract();
    void attemptBuildObject();
    void attemptPlaceLand();

    void drawGhostPlaceObjectAtCursor(ObjectType object);
    void drawGhostPlaceLandAtCursor();

    void giveStartingInventory();

    void initChestInData(BuildableObject& chest);
    void removeChestFromData(BuildableObject& chest);
    void checkChestOpenInRange();
    void handleOpenChestPositionWorldWrap(sf::Vector2f positionDelta);
    void closeChest();

    void testEnterStructure();
    void testExitStructure();

    void updateDayNightCycle(float dt);

    void drawWorld(float dt, std::vector<WorldObject*>& worldObjects, std::vector<WorldObject*>& entities);
    void drawLighting(float dt, std::vector<WorldObject*>& worldObjects, std::vector<WorldObject*>& entities);

    void updateStateTransition(float dt);
    void drawStateTransition();
    bool isStateTransitioning();
    void startChangeStateTransition(GameState newState);
    void changeState(GameState newState);

    void generateWaterNoiseTexture();

    void updateMusic(float dt);

    void drawMouseCursor();

    void handleZoom(int zoomChange);

    void handleEventsWindow(sf::Event& event);

    void toggleFullScreen();
    void handleWindowResize(sf::Vector2u newSize);

    void drawDebugMenu(float dt);

private:
    sf::RenderWindow window;
    sf::View view;
    sf::Image icon;
    bool fullScreen = true;

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

    Tween<float> floatTween;
};