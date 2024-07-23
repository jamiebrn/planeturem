#pragma once

#include <SFML/Graphics.hpp>
#include <FastNoiseLite.h>
#include <array>
#include <vector>
#include <memory>

#include "TileType.hpp"
#include "TextureManager.hpp"
#include "Camera.hpp"

#include "Object/WorldObject.hpp"
#include "Object/Tree.hpp"
#include "Object/Bush.hpp"

class Chunk
{

public:
    Chunk(sf::Vector2i worldPosition);

    void generateChunk(const FastNoiseLite& noise);

    void drawChunkTerrain(sf::RenderWindow& window);
    void drawChunkObjects(sf::RenderWindow& window);

    std::vector<WorldObject*> getObjects();

    bool isPointInChunk(sf::Vector2f position);

    inline const std::array<std::array<std::unique_ptr<WorldObject>, 8>, 8>& getObjectGrid() {return objectGrid;}

private:
    std::array<std::array<TileType, 8>, 8> groundTileGrid;
    sf::VertexArray groundVertexArray;

    std::array<std::array<std::unique_ptr<WorldObject>, 8>, 8> objectGrid;

    sf::Vector2i worldGridPosition;

};