#pragma once

#include <World/FastNoise.h>
#include <array>
#include <vector>
#include <unordered_map>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <iostream>
#include <type_traits>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/Helper.hpp"
#include "Core/Random.hpp"

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/BuildableObjectFactory.hpp"
#include "Object/ChestObject.hpp"
#include "Object/RocketObject.hpp"

#include "Object/ObjectReference.hpp"
#include "Object/StructureObject.hpp"

#include "Player/InventoryData.hpp"
#include "Player/ItemPickup.hpp"

#include "Entity/HitRect.hpp"
#include "World/TileMap.hpp"
#include "World/PathfindingEngine.hpp"

#include "Types/TileType.hpp"

#include "Data/typedefs.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"
#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"

#include "Network/PacketData/PacketDataWorld/PacketDataEntities.hpp"

#include "World/ChunkPOD.hpp"

#include "GameConstants.hpp"
#include "DebugOptions.hpp"

// Forward declaration
class Game;
class ChunkManager;
class Entity;
class ProjectileManager;

class Chunk
{

public:
    Chunk(ChunkPosition worldPosition);

    // Resets chunk to blank in a pre-generated state
    // DOES NOT RESET "WORLD POSITION" - meaning position as shown in game
    void reset(bool fullReset = false);

    // Initialisation / generation
    void generateChunk(const FastNoise& heightNoise, const FastNoise& biomeNoise, const FastNoise& riverNoise, PlanetType planetType, Game& game, ChunkManager& chunkManager,
        PathfindingEngine& pathfindingEngine, bool allowStructureGen = true, std::optional<StructureType> forceStructureType = std::nullopt,
        bool spawnEntities = true, bool initialise = true);

    // Tiles meaning tile grid, not tilemaps
    // Returns random int generator to continue to be used in generation
    RandInt generateTilesAndStructure(const FastNoise& heightNoise, const FastNoise& biomeNoise, const FastNoise& riverNoise, PlanetType planetType,
        ChunkManager& chunkManager, bool allowStructureGen = true, std::optional<StructureType> forceStructureType = std::nullopt);

    void generateObjectsAndEntities(const FastNoise& heightNoise, const FastNoise& biomeNoise, const FastNoise& riverNoise, PlanetType planetType, RandInt& randGen,
        Game& game, ChunkManager& chunkManager, bool spawnEntities = true);
    
    void spawnChunkEntities(int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise, const FastNoise& riverNoise, PlanetType planetType);

    // Generates tilemaps and calls functions to generate visual tiles and calculate collision rects
    // Called during chunk generation
    // Also used to initialise a chunk when loaded into view after being loaded through POD / save file
    void generateTilemapsAndInit(ChunkManager& chunkManager, PathfindingEngine& pathfindingEngine);

    void generateVisualEffectTiles(ChunkManager& chunkManager);

    // static int getBiomeFromNoise(float biomeNoiseValue);
    // static int getTileTypeFromNoise(float noiseValue, int biome);
    // static int getTileTypeGenerationAtPosition(int x, int y, const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType, int worldSize);

    // ALWAYS CALL THROUGH CHUNK MANAGER WHEN PLACING SINGLE TILE
    // to ensure adjacent chunk tilemaps are updated accordingly
    void setTile(int tileMap, pl::Vector2<int> position,
        TileMap* upTiles = nullptr, TileMap* downTiles = nullptr, TileMap* leftTiles = nullptr, TileMap* rightTiles = nullptr,
        bool graphicsUpdate = true);
    void setTile(int tileMap, pl::Vector2<int> position, Chunk* upChunk, Chunk* downChunk, Chunk* leftChunk, Chunk* rightChunk, bool graphicsUpdate = true);

    // Update / refresh tilemap due to changes in another chunk
    // Pass in chunk position difference relative to modified chunk to update correct edge of tile map
    void updateTileMap(int tileMap, int xRel, int yRel, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);

    // Get (terrain) tile at position in chunk
    int getTileType(pl::Vector2<int> position) const;

    TileMap* getTileMap(int tileMap);


    // Drawing
    void drawChunkTerrain(pl::RenderTarget& window, const Camera& camera, float time, int worldSize);
    void drawChunkTerrainVisual(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, PlanetType planetType, int worldSize, float time);
    void drawChunkWater(pl::RenderTarget& window, const Camera& camera, ChunkManager& chunkManager);

    // Get vector of chunk object/entities for drawing
    std::vector<WorldObject*> getObjects();
    std::vector<WorldObject*> getEntities();


    // -- Object handling -- //
    // Update all objects
    void updateChunkObjects(Game& game, float dt, int worldSize, ChunkManager& chunkManager, PathfindingEngine& pathfindingEngine);
    
    // Get object (optional) at position
    template <class T = BuildableObject>
    T* getObject(pl::Vector2<int> position);

    // Sets object (and object references if size > (1, 1), across chunks)
    void setObject(pl::Vector2<int> position, ObjectType objectType, Game& game, ChunkManager& chunkManager, PathfindingEngine* pathfindingEngine,
        bool recalculateCollision = true, bool calledWhileGenerating = false);

    // Deletes object (including object references if size > (1, 1), across chunks)
    void deleteObject(pl::Vector2<int> position, ChunkManager& chunkManager, PathfindingEngine& pathfindingEngine);

    // Delete single object (not reference)
    void deleteSingleObject(pl::Vector2<int> position, ChunkManager& chunkManager, PathfindingEngine& pathfindingEngine);

    // Create object reference at position
    void setObjectReference(const ObjectReference& objectReference, pl::Vector2<int> tile, ChunkManager& chunkManager, PathfindingEngine& pathfindingEngine);

    // Tests whether object can be placed, taking into account size and attributes (e.g. water placeable) at position
    bool canPlaceObject(pl::Vector2<int> position, ObjectType objectType, int worldSize, ChunkManager& chunkManager);


    // -- Entity handling -- //
    void updateChunkEntities(float dt, int worldSize, ProjectileManager* projectileManager, ChunkManager& chunkManager, Game* game, bool networkUpdateOnly);

    void testEntityHitCollision(const std::vector<HitRect>& hitRects, ChunkManager& chunkManager, Game& game, float gameTime);

    void moveEntityToChunk(std::unique_ptr<Entity> entity);

    // Cursor position IS IN WORLD SPACE
    Entity* getSelectedEntity(pl::Vector2f cursorPos);

    std::vector<PacketDataEntities::EntityPacketData> getEntityPacketDatas();
    void loadEntityPacketData(const PacketDataEntities::EntityPacketData& packetData);
    void clearEntities();


    // -- Item pickups -- //
    uint64_t addItemPickup(const ItemPickup& itemPickup, std::optional<uint64_t> idOverride = std::nullopt);

    std::optional<ItemPickupReference> getCollidingItemPickup(const CollisionRect& playerCollision, float gameTime, int worldSize);
    void deleteItemPickup(uint64_t id);

    ItemPickup* getItemPickup(uint64_t id);

    std::vector<WorldObject*> getItemPickups();

    void overwriteItemPickupsMap(const std::unordered_map<uint64_t, ItemPickup>& itemPickups);
    const std::unordered_map<uint64_t, ItemPickup>& getItemPickupsMap();


    // -- Collision -- //
    // Calculate all collision rects (should be called after modifying terrain/objects etc)
    // If pathfinding engine passed in, update with collision accordingly
    void recalculateCollisionRects(ChunkManager& chunkManager, PathfindingEngine* pathfindingEngine);

    // Get collision rects (previously calculated)
    std::vector<CollisionRect*> getCollisionRects();

    // Used for collision with world
    bool collisionRectStaticCollisionX(CollisionRect& collisionRect, float dx, int worldSize);
    bool collisionRectStaticCollisionY(CollisionRect& collisionRect, float dy, int worldSize);

    // Test collision rect against entities (used for testing when placing / destroying objects)
    bool isCollisionRectCollidingWithEntities(const CollisionRect& collisionRect, int worldSize);


    // -- Land -- //
    // Check whether land can be placed
    bool canPlaceLand(pl::Vector2<int> tile);

    // Place land and update visual tiles for chunk
    // Requires worldSize, noise, and chunk manager as regenerates visual tiles and collision rects
    // ONLY CALL IF CHECKED WHETHER CAN PLACE FIRST, DOES NOT RECHECK
    void placeLand(pl::Vector2<int> tile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType, ChunkManager& chunkManager,
        PathfindingEngine& pathfindingEngine);


    // -- Structures -- //
    bool isPlayerInStructureEntrance(pl::Vector2f playerPos);

    StructureObject* getStructureObject();


    // Save / load
    ChunkPOD getChunkPOD(bool includeEntities = true);
    void loadFromChunkPOD(const ChunkPOD& pod, Game& game, ChunkManager& chunkManager);
    bool wasGeneratedFromPOD();
    

    // Misc
    void setWorldPosition(pl::Vector2f position, ChunkManager& chunkManager);
    pl::Vector2f getWorldPosition();

    bool getContainsWater();
    bool hasBeenModified();

    ChunkPosition getChunkPosition();

    // bool isPointInChunk(pl::Vector2f position);

    // inline std::array<std::array<std::optional<BuildableObject>, 8>, 8>& getObjectGrid() {return objectGrid;}

    // May return nullptr
    static const BiomeGenData* getBiomeGenAtWorldTile(pl::Vector2<int> worldTile, int worldSize, const FastNoise& biomeNoise, PlanetType planetType);

    static const TileGenData* getTileGenAtWorldTile(pl::Vector2<int> worldTile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise, const FastNoise& riverNoise,
        PlanetType planetType);

    static ObjectType getRandomObjectToSpawnAtWorldTile(pl::Vector2<int> worldTile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise,
        const FastNoise& riverNoise, RandInt& randGen, PlanetType planetType);

    static EntityType getRandomEntityToSpawnAtWorldTile(pl::Vector2<int> worldTile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise,
        const FastNoise& riverNoise, PlanetType planetType);

private:
    void generateRandomStructure(int worldSize, const FastNoise& biomeNoise, RandInt& randGen, PlanetType planetType, bool allowStructureGen,
        std::optional<StructureType> forceStructureType);

private:
    // 0 reserved for water / no tile
    std::array<std::array<uint16_t, 8>, 8> groundTileGrid;
    // sf::VertexArray groundVertexArray;
    std::map<int, TileMap> tileMaps;

    // Stores visual tile types, e.g. cliffs
    std::array<std::array<TileType, 8>, 8> visualTileGrid;

    // std::array<std::array<std::optional<BuildableObject>, 8>, 8> objectGrid;
    std::array<std::array<std::unique_ptr<BuildableObject>, 8>, 8> objectGrid;
    std::vector<std::unique_ptr<Entity>> entities;

    std::unordered_map<uint64_t, ItemPickup> itemPickups;
    uint64_t itemPickupCounter; // used as ID for pickups

    // Stores collision rects for terrain and objects (NOT ENTITIES)
    std::vector<CollisionRect> collisionRects;

    // Stores chunk position in chunkmanager hashmap (NOT actual world position)
    ChunkPosition chunkPosition;
    
    // Stores ACTUAL position in world
    pl::Vector2f worldPosition;

    // Structure data, if structure is in chunk
    std::optional<StructureObject> structureObject = std::nullopt;

    bool containsWater;

    bool modified;

    // Is true if this chunk was loaded from POD / save file
    // Used to determine whether to generate tilemaps for the chunk when loaded
    bool generatedFromPOD = false;

};

template <class T>
inline T* Chunk::getObject(pl::Vector2<int> position)
{
    BuildableObject* objectPtr = objectGrid[position.y][position.x].get();
    if (!objectPtr)
    {
        return nullptr;
    }

    if constexpr (std::is_same_v<T, BuildableObject>)
    {
        return objectPtr;
    }

    return dynamic_cast<T*>(objectPtr);
}