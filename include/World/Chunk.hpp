#pragma once

#include <SFML/Graphics.hpp>
#include <World/FastNoiseLite.h>
#include <array>
#include <vector>
#include <memory>

#include "Types/TileType.hpp"
#include "Core/TextureManager.hpp"
#include "Core/Camera.hpp"
#include "Core/CollisionRect.hpp"

#include "Object/WorldObject.hpp"
#include "Object/Tree.hpp"
#include "Object/Bush.hpp"

#include "Object/ObjectBuilder.hpp"

class Chunk
{

public:
    Chunk(sf::Vector2i worldPosition);

    void generateChunk(const FastNoiseLite& noise);

    void drawChunkTerrain(sf::RenderWindow& window);
    void drawChunkObjects(sf::RenderWindow& window);

    void updateChunkObjects(float dt);
    std::vector<WorldObject*> getObjects();
    void setObject(sf::Vector2i position, ObjectType objectType);

    bool canPlaceObject(sf::Vector2i selected_tile);

    std::vector<std::unique_ptr<CollisionRect>> getCollisionRects();

    bool isPointInChunk(sf::Vector2f position);

    inline const std::array<std::array<std::unique_ptr<WorldObject>, 8>, 8>& getObjectGrid() {return objectGrid;}

private:
    std::array<std::array<TileType, 8>, 8> groundTileGrid;
    sf::VertexArray groundVertexArray;

    std::array<std::array<std::unique_ptr<WorldObject>, 8>, 8> objectGrid;

    sf::Vector2i worldGridPosition;

};