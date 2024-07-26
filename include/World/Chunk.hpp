#pragma once

#include <SFML/Graphics.hpp>
#include <World/FastNoiseLite.h>
#include <array>
#include <vector>
#include <memory>
#include <iostream>

#include "Core/TextureManager.hpp"
#include "Core/Camera.hpp"
#include "Core/CollisionRect.hpp"
#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/OccupiedTileObject.hpp"
#include "Object/ObjectReference.hpp"
#include "World/ChunkManager.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Types/TileType.hpp"

class Chunk
{

public:
    Chunk(sf::Vector2i worldPosition);

    void generateChunk(const FastNoiseLite& noise);

    void drawChunkTerrain(sf::RenderWindow& window);
    void drawChunkObjects(sf::RenderWindow& window);

    void updateChunkObjects(float dt);
    std::vector<WorldObject*> getObjects();

    void setObject(sf::Vector2i position, unsigned int objectType);
    void deleteObject(sf::Vector2i position);

    void setObjectReference(const ObjectReference& objectReference, sf::Vector2i tile);

    bool canPlaceObject(sf::Vector2i selected_tile);

    std::vector<std::unique_ptr<CollisionRect>> getCollisionRects();

    bool isPointInChunk(sf::Vector2f position);

    inline const std::array<std::array<std::unique_ptr<BuildableObject>, 8>, 8>& getObjectGrid() {return objectGrid;}

private:
    std::array<std::array<TileType, 8>, 8> groundTileGrid;
    sf::VertexArray groundVertexArray;

    std::array<std::array<std::unique_ptr<BuildableObject>, 8>, 8> objectGrid;

    sf::Vector2i worldGridPosition;

};