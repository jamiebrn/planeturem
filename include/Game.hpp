#pragma once

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <extlib/steam/steam_api.h>
#include <chrono>

#include <World/FastNoise.h>
#include <Core/json.hpp>
#include <cmath>
#include <array>
#include <unordered_map>
#include <iostream>
#include <string>
#include <memory>

#include <Graphics/TextDrawData.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/VertexArray.hpp>
#include <Graphics/SpriteBatch.hpp>
#include <Window.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/Sounds.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Helper.hpp"
#include "Core/Tween.hpp"
#include "Core/InputManager.hpp"

#include "World/ChunkManager.hpp"
#include "World/ChestDataPool.hpp"
#include "World/TileMap.hpp"

#include "World/Room.hpp"
#include "World/RoomPool.hpp"

#include "World/DayCycleManager.hpp"
#include "World/LightingEngine.hpp"
#include "World/PathfindingEngine.hpp"
#include "World/LandmarkManager.hpp"
#include "World/WeatherSystem.hpp"
#include "World/WorldData.hpp"
#include "World/RoomDestinationData.hpp"

#include "Player/Player.hpp"
#include "Player/PlayerData.hpp"
#include "Player/LocationState.hpp"
#include "Player/Cursor.hpp"
#include "Player/InventoryData.hpp"
#include "Player/ItemPickup.hpp"
#include "Player/Achievements.hpp"

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/ChestObject.hpp"
#include "Object/RocketObject.hpp"
#include "Object/NPCObject.hpp"
#include "Object/LandmarkObject.hpp"
#include "Object/StructureObject.hpp"
#include "Object/ParticleSystem.hpp"

#include "Entity/Boss/BossManager.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"

#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ItemData.hpp"
#include "Data/RecipeData.hpp"
#include "Data/EntityData.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/ArmourData.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/RecipeDataLoader.hpp"
#include "Data/EntityDataLoader.hpp"
#include "Data/ToolDataLoader.hpp"
#include "Data/PlanetGenDataLoader.hpp"
#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"
#include "Data/ArmourDataLoader.hpp"

#include "Types/GameState.hpp"
#include "Types/WorldMenuState.hpp"

#include "GUI/MainMenuGUI.hpp"
#include "GUI/InventoryGUI.hpp"
#include "GUI/HealthGUI.hpp"
#include "GUI/TravelSelectGUI.hpp"
#include "GUI/LandmarkSetGUI.hpp"
#include "GUI/NPCInteractionGUI.hpp"
#include "GUI/HitMarkers.hpp"

#include "Network/Packet.hpp"
#include "Network/IPacketData.hpp"
#include "Network/PacketData/PacketDataIncludes.hpp"

#include "Network/NetworkHandler.hpp"

#include "IO/GameSaveIO.hpp"

#include "DebugOptions.hpp"

class Game
{
public:
    bool initialise();

    void deinit();

    void run();

public:
    // Chest
    void openChest(ChestObject& chest, std::optional<LocationState> chestLocationState, bool initiatedClientSide);
    void openChestForClient(PacketDataChestOpened packetData);
    void openChestFromHost(const PacketDataChestOpened& packetData);
    void openedChestDataModified();
    void closeChest(std::optional<ObjectReference> chestObjectRef = std::nullopt, std::optional<LocationState> chestLocationState = std::nullopt,
        bool sentFromHost = false, std::optional<uint64_t> userId = std::nullopt);
    uint16_t getOpenChestID();

    // Rocket
    void enterRocket(RocketObject& rocket);
    void exitRocket();
    void enterIncomingRocket(RocketObject& rocket);
    void rocketFinishedUp(RocketObject& rocket);
    void rocketFinishedDown(RocketObject& rocket);

    // NPC
    void interactWithNPC(NPCObject& npc);

    // Landmark
    void landmarkPlaced(const LandmarkObject& landmark, bool createGUI);
    void landmarkDestroyed(const LandmarkObject& landmark);

    // Melee combat
    void testMeleeCollision(const std::vector<HitRect>& hitRects);

    // Item pickups created alert
    void itemPickupsCreated(const std::vector<ItemPickupReference>& itemPickupsCreated, std::optional<LocationState> pickupsLocationState);

    void drawWorld(pl::Framebuffer& renderTexture, float dt, std::vector<WorldObject*>& worldObjects, WorldData& worldData, const Camera& cameraArg);

    void joinWorld(const PacketDataJoinInfo& joinInfo);
    void quitWorld();

    // Attempts to load planet from disk if required - initialises new planet if not available
    bool loadPlanet(PlanetType planetType);

    void hitObject(ChunkPosition chunk, pl::Vector2<int> tile, int damage, std::optional<PlanetType> planetType = std::nullopt,
        bool sentFromHost = false, std::optional<uint64_t> userId = std::nullopt);
    void buildObject(ChunkPosition chunk, pl::Vector2<int> tile, ObjectType objectType, std::optional<PlanetType> planetType = std::nullopt,
        bool sentFromHost = false, bool builtByPlayer = true);
    void destroyObjectFromHost(ChunkPosition chunk, pl::Vector2<int> tile, std::optional<PlanetType> planetType);

    // Networking
    void joinedLobby(bool requiresNameInput);

    void handleChunkRequestsFromClient(const PacketDataChunkRequests& chunkRequests, const SteamNetworkingIdentity& client);
    void handleChunkDataFromHost(const PacketDataChunkDatas& chunkDataPacket);

    ObjectReference setupPlanetTravel(PlanetType planetType, const LocationState& currentLocation, ObjectReference rocketObjectUsed, std::optional<uint64_t> clientID);
    void travelToPlanetFromHost(const PacketDataPlanetTravelReply& planetTravelReplyPacket);

    std::optional<uint32_t> initialiseStructureOrGet(PlanetType planetType, ChunkPosition chunk, pl::Vector2f* entrancePos, RoomType* roomType);
    void enterStructureFromHost(PlanetType planetType, ChunkPosition chunk, uint32_t structureID, pl::Vector2f entrancePos, RoomType roomType);

    PlayerData createPlayerData();


    // Misc

    inline bool getIsDay() {return isDay;}
    DayCycleManager& getDayCycleManager(bool overrideMenuSwap = false);

    ChunkManager& getChunkManager(std::optional<PlanetType> planetTypeOverride = std::nullopt);
    ProjectileManager& getProjectileManager(std::optional<PlanetType> planetTypeOverride = std::nullopt);
    BossManager& getBossManager(std::optional<PlanetType> planetTypeOverride = std::nullopt);
    LandmarkManager& getLandmarkManager(std::optional<PlanetType> planetTypeOverride = std::nullopt);
    RoomPool& getStructureRoomPool(std::optional<PlanetType> planetTypeOverride = std::nullopt);
    Room& getRoomDestination(std::optional<RoomType> roomDestOverride = std::nullopt);
    ChestDataPool& getChestDataPool(std::optional<LocationState> locationState = std::nullopt);
    bool isLocationStateInitialised(const LocationState& locationState);

    inline const Camera& getCamera() {return camera;}

    inline Player& getPlayer() {return player;}
    inline InventoryData& getInventory() {return inventory;}

    inline float getGameTime() {return gameTime;}
    inline void setGameTime(float gameTime) {this->gameTime = gameTime;}

    inline int getPlanetSeed() {return planetSeed;}

private:

    // -- Main Menu -- //

    void runMainMenu(float dt);


    // -- Main Game -- //

    void runInGame(float dt);


    // -- On Planet -- //

    void updateOnPlanet(float dt);
    void drawOnPlanet(float dt);

    // Used when host
    void updateActivePlanets(float dt);

    void drawLighting(float dt, std::vector<WorldObject*>& worldObjects);
    void drawLandmarks();

    void testEnterStructure();
    void testExitStructure();


    // -- In Room -- //

    void updateInRoom(float dt, Room& room, bool inStructure);
    void drawInRoom(float dt, const Room& room);


    // -- Tools / interactions -- //

    void changePlayerTool();
    void attemptUseTool();
    void attemptUseToolPickaxe();
    void attemptUseToolFishingRod();
    void attemptUseToolWeapon();

    void catchRandomFish(pl::Vector2<int> fishedTile);
    
    void attemptObjectInteract();
    void attemptBuildObject();
    void attemptPlaceLand();

    void attemptUseBossSpawn();

    void attemptUseConsumable();

    void drawGhostPlaceObjectAtCursor(ObjectType object);
    void drawGhostPlaceLandAtCursor();

    // Returns pointer (may be null) to selected / hovered object
    // Depending on game state, will check chunks / room
    BuildableObject* getSelectedObjectFromChunkOrRoom();

    // For chunks, will use chunk and tile from object reference
    // For rooms, will use tile from object reference
    template <class T = BuildableObject>
    T* getObjectFromLocation(ObjectReference objectReference, const LocationState& objectLocationState);


    // -- Inventory / Chests -- //

    void handleInventoryClose();

    void checkChestOpenInRange();
    // void handleOpenChestPositionWorldWrap(pl::Vector2f positionDelta);


    // -- Planet travelling -- //

    void travelToDestination();
    void travelToPlanet(PlanetType planetType, ObjectReference newRocketObjectReference);
    void deleteObjectSynced(ObjectReference objectReference, PlanetType planetType);
    
    void travelToRoomDestination(RoomType destinationRoomType);

    // Returns spawn chunk
    ChunkPosition initialiseNewPlanet(PlanetType planetType);


    // -- Game State / Transition -- //

    void updateStateTransition(float dt);
    void drawStateTransition();
    bool isStateTransitioning();
    void startChangeStateTransition(GameState newState);
    void changeState(GameState newState);
    // Only use to prevent game state changing default behaviour
    void overrideState(GameState newState);
    

    // -- Save / load -- //

    void startNewGame(int seed);
    bool saveGame(bool gettingInRocket = false);
    bool loadGame(const SaveFileSummary& saveFileSummary);

    void initialiseWorldData(PlanetType planetType);
    
    void saveOptions();
    void loadOptions();
    

    // -- Window -- //

    void handleZoom(int zoomChange);

    void handleEventsWindow(const SDL_Event& event);

    void toggleFullScreen();
    void handleWindowResize(pl::Vector2<uint32_t> newSize);


    // -- Misc -- //

    void generateWaterNoiseTexture();

    void updateMusic(float dt);

    void drawMouseCursor();

    void drawControllerGlyphs(const std::vector<std::pair<InputAction, std::string>>& actionStrings);

    #if (!RELEASE_BUILD)
    void drawDebugMenu(float dt);
    #endif


private:
    pl::Window window;
    pl::Image icon;

    SaveFileSummary currentSaveFileSummary;
    float saveSessionPlayTime;

    pl::SpriteBatch spriteBatch;
    pl::Framebuffer worldTexture;

    bool steamInitialised;

    float gameTime;

    bool isDay;

    static constexpr float MUSIC_GAP_MIN = 20.0f;
    float musicGapTimer;
    float musicGap;

    // GUI
    MainMenuGUI mainMenuGUI;
    NPCInteractionGUI npcInteractionGUI;
    TravelSelectGUI travelSelectGUI;
    LandmarkSetGUI landmarkSetGUI;
    pl::Vector2f mouseScreenPos;

    // Game general data
    Player player;
    LocationState locationState;
    Camera camera;
    InventoryData inventory;
    InventoryData armourInventory;
    WeatherSystem weatherSystem;
    ParticleSystem particleSystem;
    DayCycleManager dayCycleManager;

    // Set planet / room dest type to -1 when selecting other
    std::unordered_map<PlanetType, WorldData> worldDatas;
    std::unordered_map<RoomType, RoomDestinationData> roomDestDatas;
    int planetSeed;

    std::unordered_map<PlanetType, ObjectReference> planetRocketUsedPositions;

    LightingEngine lightingEngine;
    int lightingTick = 0;
    bool smoothLighting = true;

    NetworkHandler networkHandler;

    std::array<pl::Texture, 2> waterNoiseTextures;

    GameState gameState;
    GameState destinationGameState;
    static constexpr float TRANSITION_STATE_FADE_TIME = 0.3f;
    float transitionGameStateTimer;

    WorldMenuState worldMenuState;

    std::unordered_map<std::string, int> nearbyCraftingStationLevels;

    // 0xFFFF chest ID reserved for no chest opened / non-initialised chest
    uint16_t openedChestID;
    ObjectReference openedChest;

    // Structure
    pl::Vector2f structureEnteredPos;

    // Rocket
    ObjectReference rocketEnteredReference;
    LocationState destinationLocationState;
    bool travelTrigger = false;

    Tween<float> floatTween;

};