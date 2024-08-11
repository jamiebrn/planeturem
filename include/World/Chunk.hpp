#pragma once

#include <SFML/Graphics.hpp>
#include <World/FastNoise.h>
#include <array>
#include <vector>
#include <memory>
#include <optional>
#include <iostream>

#include "Core/TextureManager.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/ObjectReference.hpp"
#include "Entity/Entity.hpp"
#include "World/ChunkManager.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Types/TileType.hpp"

#include "GameConstants.hpp"

// Forward declaration
class ChunkManager;
class Entity;

class Chunk
{

public:
    Chunk(sf::Vector2i worldPosition);

    // Initialisation
    void generateChunk(const FastNoise& noise, int worldSize, ChunkManager& chunkManager);

    void generateVisualEffectTiles(const FastNoise& noise, int worldSize, ChunkManager& chunkManager);

    static TileType getTileTypeFromNoiseHeight(float noiseValue);

    // Drawing
    void drawChunkTerrain(sf::RenderTarget& window, float time);
    void drawChunkTerrainVisual(sf::RenderTarget& window, float time);
    void drawChunkWater(sf::RenderTarget& window, float time);

    // Get vector of chunk object/entities for drawing
    std::vector<WorldObject*> getObjects();
    std::vector<WorldObject*> getEntities();

    // Get (terrain) tile at position in chunk
    TileType getTileType(sf::Vector2i position) const;


    // -- Object handling -- //
    // Update all objects
    void updateChunkObjects(float dt, ChunkManager& chunkManager);
    
    // Get object (optional) at position
    std::optional<BuildableObject>& getObject(sf::Vector2i position);

    // Sets object (and object references if size > (1, 1), across chunks)
    void setObject(sf::Vector2i position, unsigned int objectType, int worldSize, ChunkManager& chunkManager);

    // Deletes object (including object references if size > (1, 1), across chunks)
    void deleteObject(sf::Vector2i position, ChunkManager& chunkManager);

    // Create object reference at position
    void setObjectReference(const ObjectReference& objectReference, sf::Vector2i tile, ChunkManager& chunkManager);

    // Tests whether object can be placed, taking into account size and attributes (e.g. water placeable) at position
    bool canPlaceObject(sf::Vector2i position, unsigned int objectType, int worldSize, ChunkManager& chunkManager);


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


    void setWorldPosition(sf::Vector2f position);


    // bool isPointInChunk(sf::Vector2f position);

    // inline std::array<std::array<std::optional<BuildableObject>, 8>, 8>& getObjectGrid() {return objectGrid;}

private:
    std::array<std::array<TileType, 8>, 8> groundTileGrid;
    sf::VertexArray groundVertexArray;

    // Stores visual tile types, e.g. cliffs
    std::array<std::array<TileType, 8>, 8> visualTileGrid;

    std::array<std::array<std::optional<BuildableObject>, 8>, 8> objectGrid;
    std::vector<std::unique_ptr<Entity>> entities;

    // Stores collision rects for terrain and objects (NOT ENTITIES)
    std::vector<std::unique_ptr<CollisionRect>> collisionRects;

    // Stores chunk position in chunkmanager hashmap (NOT actual world position)
    sf::Vector2i worldGridPosition;
    
    // Stores ACTUAL position in world, which may differ from grid position if repeating chunks
    sf::Vector2f worldPosition;

};