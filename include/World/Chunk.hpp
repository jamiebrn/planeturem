#pragma once

#include <SFML/Graphics.hpp>
#include <World/FastNoise.h>
#include <array>
#include <vector>
#include <unordered_map>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <iostream>

#include "Core/TextureManager.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/SpriteBatch.hpp"
#include "Core/Helper.hpp"
#include "Core/Random.hpp"

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/BuildableObjectFactory.hpp"
#include "Object/ChestObject.hpp"
#include "Object/RocketObject.hpp"

#include "Object/ObjectReference.hpp"
#include "Object/StructureObject.hpp"

#include "Entity/Entity.hpp"
#include "World/ChunkManager.hpp"
#include "World/TileMap.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Types/TileType.hpp"

#include "Data/typedefs.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"
#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"

#include "World/ChunkPOD.hpp"

#include "GameConstants.hpp"
#include "DebugOptions.hpp"

// Forward declaration
class Game;
class ChunkManager;
class Entity;

class Chunk
{

public:
    Chunk(ChunkPosition worldPosition);

    // Initialisation / generation
    void generateChunk(const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType, int worldSize, ChunkManager& chunkManager);

    // Tiles meaning tile grid, not tilemaps
    // Returns random int generator to continue to be used in generation
    RandInt generateTilesAndStructure(const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType, int worldSize, ChunkManager& chunkManager);

    void generateObjectsAndEntities(const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType, RandInt& randGen,
        int worldSize, ChunkManager& chunkManager);

    // Generates tilemaps and calls functions to generate visual tiles and calculate collision rects
    // Called during chunk generation
    // Also used to initialise a chunk when loaded into view after being loaded through POD / save file
    void generateTilemapsAndInit(const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType, int worldSize, ChunkManager& chunkManager);

    void generateVisualEffectTiles(const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType, int worldSize, ChunkManager& chunkManager);

    // static int getBiomeFromNoise(float biomeNoiseValue);
    // static int getTileTypeFromNoise(float noiseValue, int biome);
    // static int getTileTypeGenerationAtPosition(int x, int y, const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType, int worldSize);

    // ALWAYS CALL THROUGH CHUNK MANAGER WHEN PLACING SINGLE TILE
    // to ensure adjacent chunk tilemaps are updated accordingly
    void setTile(int tileMap, sf::Vector2i position,
        TileMap* upTiles = nullptr, TileMap* downTiles = nullptr, TileMap* leftTiles = nullptr, TileMap* rightTiles = nullptr,
        bool graphicsUpdate = true);
    void setTile(int tileMap, sf::Vector2i position, Chunk* upChunk, Chunk* downChunk, Chunk* leftChunk, Chunk* rightChunk, bool graphicsUpdate = true);

    // Update / refresh tilemap due to changes in another chunk
    // Pass in chunk position difference relative to modified chunk to update correct edge of tile map
    void updateTileMap(int tileMap, int xRel, int yRel, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);

    // Get (terrain) tile at position in chunk
    int getTileType(sf::Vector2i position) const;

    TileMap* getTileMap(int tileMap);


    // Drawing
    void drawChunkTerrain(sf::RenderTarget& window, float time);
    void drawChunkTerrainVisual(sf::RenderTarget& window, SpriteBatch& spriteBatch, float time);
    void drawChunkWater(sf::RenderTarget& window);

    // Get vector of chunk object/entities for drawing
    std::vector<WorldObject*> getObjects();
    std::vector<WorldObject*> getEntities();


    // -- Object handling -- //
    // Update all objects
    void updateChunkObjects(Game& game, float dt, int worldSize, ChunkManager& chunkManager);
    
    // Get object (optional) at position
    BuildableObject* getObject(sf::Vector2i position);

    // Sets object (and object references if size > (1, 1), across chunks)
    void setObject(sf::Vector2i position, ObjectType objectType, int worldSize, ChunkManager& chunkManager, bool recalculateCollision = true, bool calledWhileGenerating = false);

    // Deletes object (including object references if size > (1, 1), across chunks)
    void deleteObject(sf::Vector2i position, ChunkManager& chunkManager);

    // Delete single object (not reference)
    void deleteSingleObject(sf::Vector2i position, ChunkManager& chunkManager);

    // Create object reference at position
    void setObjectReference(const ObjectReference& objectReference, sf::Vector2i tile, ChunkManager& chunkManager);

    // Tests whether object can be placed, taking into account size and attributes (e.g. water placeable) at position
    bool canPlaceObject(sf::Vector2i position, ObjectType objectType, int worldSize, ChunkManager& chunkManager);


    // -- Entity handling -- //
    void updateChunkEntities(float dt, int worldSize, ChunkManager& chunkManager);
    void moveEntityToChunk(std::unique_ptr<Entity> entity);

    // Cursor position IS IN WORLD SPACE
    Entity* getSelectedEntity(sf::Vector2f cursorPos);

    // -- Collision -- //
    // Calculate all collision rects (should be called after modifying terrain/objects etc)
    // DOES NOT INCLUDE ENTITIES
    void recalculateCollisionRects(ChunkManager& chunkManager);

    // Get collision rects (previously calculated)
    std::vector<CollisionRect*> getCollisionRects();

    // Used for collision with world
    bool collisionRectStaticCollisionX(CollisionRect& collisionRect, float dx);
    bool collisionRectStaticCollisionY(CollisionRect& collisionRect, float dy);

    // Test collision rect against entities (used for testing when placing / destroying objects)
    bool isCollisionRectCollidingWithEntities(const CollisionRect& collisionRect);


    // -- Land -- //
    // Check whether land can be placed
    bool canPlaceLand(sf::Vector2i tile);

    // Place land and update visual tiles for chunk
    // Requires worldSize, noise, and chunk manager as regenerates visual tiles and collision rects
    // ONLY CALL IF CHECKED WHETHER CAN PLACE FIRST, DOES NOT RECHECK
    void placeLand(sf::Vector2i tile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType, ChunkManager& chunkManager);


    // -- Structures -- //
    bool isPlayerInStructureEntrance(sf::Vector2f playerPos, StructureEnterEvent& enterEvent);

    bool hasStructure();


    // Save / load
    ChunkPOD getChunkPOD();
    void loadFromChunkPOD(const ChunkPOD& pod);
    bool wasGeneratedFromPOD();
    

    // Misc
    void setWorldPosition(sf::Vector2f position, ChunkManager& chunkManager);
    sf::Vector2f getWorldPosition();

    bool getContainsWater();
    bool hasBeenModified();

    // bool isPointInChunk(sf::Vector2f position);

    // inline std::array<std::array<std::optional<BuildableObject>, 8>, 8>& getObjectGrid() {return objectGrid;}

    // May return nullptr
    static const BiomeGenData* getBiomeGenAtWorldTile(sf::Vector2i worldTile, int worldSize, const FastNoise& biomeNoise, PlanetType planetType);

    static const TileGenData* getTileGenAtWorldTile(sf::Vector2i worldTile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType);

    static ObjectType getRandomObjectToSpawnAtWorldTile(sf::Vector2i worldTile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise,
        RandInt& randGen, PlanetType planetType);

    static EntityType getRandomEntityToSpawnAtWorldTile(sf::Vector2i worldTile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise,
        RandInt& randGen, PlanetType planetType);

private:
    void generateRandomStructure(int worldSize, const FastNoise& biomeNoise, RandInt& randGen, PlanetType planetType);

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

    // Stores collision rects for terrain and objects (NOT ENTITIES)
    // TODO: Stop using uniqueptr
    std::vector<std::unique_ptr<CollisionRect>> collisionRects;

    // Stores chunk position in chunkmanager hashmap (NOT actual world position)
    ChunkPosition chunkPosition;
    
    // Stores ACTUAL position in world, which may differ from grid position if repeating chunks
    sf::Vector2f worldPosition;

    // Structure data, if structure is in chunk
    std::optional<StructureObject> structureObject = std::nullopt;

    bool containsWater;

    bool modified;

    // Is true if this chunk was loaded from POD / save file
    // Used to determine whether to generate tilemaps for the chunk when loaded
    bool generatedFromPOD = false;

};