#pragma once

#include <SFML/Graphics.hpp>
// #include <World/FastNoiseLite.h>
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
#include "Object/OccupiedTileObject.hpp"
#include "Object/ObjectReference.hpp"
#include "World/ChunkManager.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Types/TileType.hpp"

// Forward declaration
class ChunkManager;

class Chunk
{

public:
    Chunk(sf::Vector2i worldPosition);

    void generateChunk(const FastNoise& noise, int worldSize);

    void drawChunkTerrain(sf::RenderWindow& window);
    void drawChunkObjects(sf::RenderWindow& window);

    void updateChunkObjects(float dt, ChunkManager& chunkManager);
    std::vector<WorldObject*> getObjects();

    std::optional<BuildableObject>& getObject(sf::Vector2i position);
    TileType getTileType(sf::Vector2i position) const;

    void setObject(sf::Vector2i position, unsigned int objectType, int worldSize, ChunkManager& chunkManager);
    void deleteObject(sf::Vector2i position, ChunkManager& chunkManager);

    void setObjectReference(const ObjectReference& objectReference, sf::Vector2i tile);

    bool canPlaceObject(sf::Vector2i position, unsigned int objectType, int worldSize, ChunkManager& chunkManager);

    std::vector<std::unique_ptr<CollisionRect>> getCollisionRects(ChunkManager& chunkManager);

    void setWorldPosition(sf::Vector2f position);

    bool isPointInChunk(sf::Vector2f position);

    // inline std::array<std::array<std::optional<BuildableObject>, 8>, 8>& getObjectGrid() {return objectGrid;}

private:
    std::array<std::array<TileType, 8>, 8> groundTileGrid;
    sf::VertexArray groundVertexArray;

    std::array<std::array<std::optional<BuildableObject>, 8>, 8> objectGrid;

    // Stores chunk position in chunkmanager hashmap (NOT actual world position)
    sf::Vector2i worldGridPosition;
    
    // Stores ACTUAL position in world, which may differ from grid position if repeating chunks
    sf::Vector2f worldPosition;

};