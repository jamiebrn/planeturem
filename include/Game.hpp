#pragma once

#include <SFML/Graphics.hpp>
#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui-SFML.h>
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

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/Sounds.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Helper.hpp"
#include "Core/Tween.hpp"
#include "Core/SpriteBatch.hpp"
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
#include "Network/PacketDataJoinInfo.hpp"
#include "Network/PacketDataPlayerData.hpp"
#include "Network/PacketDataPlayerCharacterInfo.hpp"
#include "Network/PacketDataServerInfo.hpp"
#include "Network/PacketDataObjectHit.hpp"
#include "Network/PacketDataObjectBuilt.hpp"
#include "Network/PacketDataObjectDestroyed.hpp"
#include "Network/PacketDataItemPickupsCreated.hpp"
#include "Network/PacketDataItemPickupDeleted.hpp"
#include "Network/PacketDataItemPickupsCreateRequest.hpp"
#include "Network/PacketDataInventoryAddItem.hpp"
#include "Network/PacketDataChestOpened.hpp"
#include "Network/PacketDataChestClosed.hpp"
#include "Network/PacketDataChestDataModified.hpp"
#include "Network/PacketDataChunkDatas.hpp"
#include "Network/PacketDataChunkRequests.hpp"
#include "Network/PacketDataPlanetTravelRequest.hpp"
#include "Network/PacketDataPlanetTravelReply.hpp"

#include "Network/NetworkHandler.hpp"

#include "IO/GameSaveIO.hpp"

#include "DebugOptions.hpp"

class Game
{
public:
    bool initialise();

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

    void drawWorld(sf::RenderTexture& renderTexture, float dt, std::vector<WorldObject*>& worldObjects, WorldData& worldData, const Camera& cameraArg);

    void joinWorld(const PacketDataJoinInfo& joinInfo);
    void quitWorld();

    void hitObject(ChunkPosition chunk, sf::Vector2i tile, int damage, std::optional<PlanetType> planetType = std::nullopt,
        bool sentFromHost = false, std::optional<uint64_t> userId = std::nullopt);
    void buildObject(ChunkPosition chunk, sf::Vector2i tile, ObjectType objectType, std::optional<PlanetType> planetType = std::nullopt,
        bool sentFromHost = false);
    void destroyObjectFromHost(ChunkPosition chunk, sf::Vector2i tile, std::optional<PlanetType> planetType);

    // Networking
    void joinedLobby(bool requiresNameInput);

    void handleChunkRequestsFromClient(const PacketDataChunkRequests& chunkRequests, const SteamNetworkingIdentity& client);
    void handleChunkDataFromHost(const PacketDataChunkDatas& chunkDataPacket);

    void travelToPlanetFromHost(const PacketDataPlanetTravelReply& planetTravelReplyPacket);

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
    ChestDataPool& getChestDataPool(std::optional<PlanetType> planetTypeOverride = std::nullopt, std::optional<RoomType> roomDestOverride = std::nullopt);
    bool isLocationStateInitialised(const LocationState& locationState);

    inline const Camera& getCamera() {return camera;}

    inline Player& getPlayer() {return player;}
    inline InventoryData& getInventory() {return inventory;}

    inline float getGameTime() {return gameTime;}
    inline void setGameTime(float gameTime) {this->gameTime = gameTime;}

    inline int getPlanetSeed() {return planetSeed;}

private:

    // test
    void runFeatureTest();

    // -- Main Menu -- //

    void runMainMenu(float dt);


    // -- Main Game -- //

    void runInGame(float dt);


    // -- On Planet -- //

    void updateOnPlanet(float dt);
    void drawOnPlanet(float dt);

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

    void catchRandomFish(sf::Vector2i fishedTile);
    
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
    // void handleOpenChestPositionWorldWrap(sf::Vector2f positionDelta);


    // -- Planet travelling -- //

    void travelToDestination();
    void deleteUsedRocket(ObjectReference rocketObjectReference, PlanetType planetType);
    ObjectReference setupPlanetTravel(PlanetType planetType, ObjectReference rocketUsedObjectReference, std::optional<uint64_t> clientID);
    void handleRocketCleanup(PlanetType currentPlanetType, RoomType currentRoomDestType, ObjectReference rocketObjectReference);
    void travelToPlanet(PlanetType planetType, ObjectReference newRocketObjectReference);

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
    bool loadPlanet(PlanetType planetType);

    void initialiseWorldData(PlanetType planetType);
    
    void saveOptions();
    void loadOptions();
    

    // -- Window -- //

    void handleZoom(int zoomChange);

    void handleEventsWindow(sf::Event& event);
    void handleSDLEvents();

    void toggleFullScreen();
    void handleWindowResize(sf::Vector2u newSize);


    // -- Misc -- //

    void generateWaterNoiseTexture();

    void updateMusic(float dt);

    void drawMouseCursor();

    void drawControllerGlyphs(const std::vector<std::pair<InputAction, std::string>>& actionStrings);

    #if (!RELEASE_BUILD)
    void drawDebugMenu(float dt);
    #endif


private:
    sf::RenderWindow window;
    SDL_Window* sdlWindow = nullptr;
    sf::View view;
    sf::Image icon;
    bool fullScreen = true;

    SaveFileSummary currentSaveFileSummary;
    float saveSessionPlayTime;

    SpriteBatch spriteBatch;
    sf::RenderTexture worldTexture;

    bool steamInitialised;

    sf::Clock clock;
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
    sf::Vector2f mouseScreenPos;

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
    // sf::Vector2f openedChestPos;

    // Structure
    sf::Vector2f structureEnteredPos;

    // Rocket
    ObjectReference rocketEnteredReference;
    LocationState destinationLocationState;
    bool travelTrigger = false;

    Tween<float> floatTween;
};