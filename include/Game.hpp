#pragma once

#include <SFML/Graphics.hpp>
#include <SDL2/SDL.h>
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

#include "Player/Player.hpp"
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
#include "Network/PacketDataPlayerInfo.hpp"
#include "Network/PacketDataObjectHit.hpp"
#include "Network/PacketDataObjectBuilt.hpp"
#include "Network/PacketDataItemPickupsCreated.hpp"
#include "Network/PacketDataItemPickupDeleted.hpp"
#include "Network/PacketDataInventoryAddItem.hpp"
#include "Network/PacketDataChunkDatas.hpp"
#include "Network/PacketDataChunkRequests.hpp"

#include "IO/GameSaveIO.hpp"

#include "DebugOptions.hpp"

class Game
{
public:
    bool initialise();

    void run();

public:
    // Chest
    void openChest(ChestObject& chest);
    void closeChest();
    uint16_t getOpenChestID();
    ChestDataPool& getChestDataPool();

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
    void itemPickupsCreated(const std::vector<ItemPickupReference>& itemPickupsCreated);

    void drawWorld(sf::RenderTexture& renderTexture, float dt, std::vector<WorldObject*>& worldObjects, ChunkManager& chunkManagerArg, const Camera& cameraArg);

    inline bool getIsDay() {return isDay;}
    const DayCycleManager& getDayCycleManager();

    inline const ChunkManager& getChunkManager() {return chunkManager;}

    inline const Camera& getCamera() {return camera;}

    inline const Player& getPlayer() {return player;}

    inline float getGameTime() {return gameTime;}

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

    void hitObject(ChunkPosition chunk, sf::Vector2i tile, int damage, bool sentFromHost = false, std::optional<uint64_t> userId = std::nullopt);
    void buildObject(ChunkPosition chunk, sf::Vector2i tile, ObjectType objectType, bool sentFromHost = false);

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
    BuildableObject* getObjectFromChunkOrRoom(ObjectReference objectReference);


    // -- Inventory / Chests -- //

    void giveStartingInventory();

    void handleInventoryClose();

    void checkChestOpenInRange();
    void handleOpenChestPositionWorldWrap(sf::Vector2f positionDelta);


    // -- Planet travelling -- //

    void travelToDestination();
    void travelToPlanet(PlanetType planetType);
    void travelToRoomDestination(RoomType destinationRoomType);

    void initialiseNewPlanet(PlanetType planetType, bool placeRocket = false);


    // -- Game State / Transition -- //

    void updateStateTransition(float dt);
    void drawStateTransition();
    bool isStateTransitioning();
    void startChangeStateTransition(GameState newState);
    void changeState(GameState newState);
    // Only use to prevent game state changing default behaviour
    void overrideState(GameState newState);
    

    // -- Save / load -- //

    void startNewGame(int seed, std::optional<std::string> overridePlanetName = std::nullopt);
    bool saveGame(bool gettingInRocket = false);
    bool loadGame(const SaveFileSummary& saveFileSummary);
    bool loadPlanet(PlanetType planetType);

    void saveOptions();
    void loadOptions();

    void quitWorld();


    // -- Multiplayer -- //

    void createLobby();
    void callbackLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);
    void leaveLobby();
    void joinWorld(const PacketDataJoinInfo& joinInfo);
    STEAM_CALLBACK(Game, callbackLobbyJoinRequested, GameLobbyJoinRequested_t);
    STEAM_CALLBACK(Game, callbackLobbyEnter, LobbyEnter_t);
    STEAM_CALLBACK(Game, callbackLobbyUpdated, LobbyChatUpdate_t);
    STEAM_CALLBACK(Game, callbackMessageSessionRequest, SteamNetworkingMessagesSessionRequest_t);

    void registerNetworkPlayer(uint64_t id, bool notify = true);
    void deleteNetworkPlayer(uint64_t id);

    void receiveMessages();
    void sendHostMessages();
    EResult sendPacketToClients(const Packet& packet, int nSendFlags, int nRemoteChannel);
    void handleChunkRequestsFromClient(const PacketDataChunkRequests& chunkRequests, const SteamNetworkingIdentity& client);
    
    void sendClientMessages();
    EResult sendPacketToHost(const Packet& packet, int nSendFlags, int nRemoteChannel);
    void handleChunkDatasFromHost(const PacketDataChunkDatas& chunkDatas);
    void requestChunksFromHost(const std::vector<ChunkPosition>& chunks);


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
    Camera camera;
    InventoryData inventory;
    InventoryData armourInventory;
    ChunkManager chunkManager;
    WeatherSystem weatherSystem;
    ProjectileManager projectileManager;
    ProjectileManager enemyProjectileManager;
    BossManager bossManager;
    LandmarkManager landmarkManager;
    ParticleSystem particleSystem;
    DayCycleManager dayCycleManager;
    Room roomDestination;

    LightingEngine lightingEngine;
    int lightingTick = 0;
    bool smoothLighting = true;

    // Multiplayer
    bool multiplayerGame = false;
    uint64_t steamLobbyId = 0;
    bool isLobbyHost = false;
    uint64_t lobbyHost;
    CCallResult<Game, LobbyCreated_t> m_SteamCallResultCreateLobby;
    std::unordered_map<uint64_t, Player> networkPlayers;

    std::array<sf::Texture, 2> waterNoiseTextures;

    GameState gameState;
    GameState destinationGameState;
    static constexpr float TRANSITION_STATE_FADE_TIME = 0.3f;
    float transitionGameStateTimer;

    WorldMenuState worldMenuState;

    std::unordered_map<std::string, int> nearbyCraftingStationLevels;

    // 0xFFFF chest ID reserved for no chest opened / non-initialised chest
    uint16_t openedChestID;
    // ObjectReference openedChest;
    sf::Vector2f openedChestPos;
    ChestDataPool chestDataPool;

    // Structure
    uint32_t structureEnteredID;
    sf::Vector2f structureEnteredPos;
    RoomPool structureRoomPool;

    // Rocket
    ObjectReference rocketEnteredReference;
    PlanetType destinationPlanet;
    RoomType destinationRoom;
    bool travelTrigger = false;

    Tween<float> floatTween;
};