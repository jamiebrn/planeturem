#pragma once

#include <SFML/Graphics.hpp>
#include <array>

#include "Core/TextureManager.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/Camera.hpp"
#include "Object/BuildableObject.hpp"
#include "Entity/Entity.hpp"
#include "World/ChunkPosition.hpp"
#include "World/ChunkManager.hpp"
#include "GUI/BuildGUI.hpp"

struct CursorCornerPosition
{
    sf::Vector2f worldPositionDestination = {0, 0};
    sf::Vector2f worldPosition = {0, 0};
};

class Cursor
{
    Cursor() = delete;

public:
    static void updateTileCursor(sf::RenderWindow& window, float dt, bool buildMenuOpen, int worldSize, ChunkManager& chunkManager);

    static void drawTileCursor(sf::RenderWindow& window);
    static void drawDynamicCursor(sf::RenderWindow& window);

    static ChunkPosition getSelectedChunk(int worldSize);
    static sf::Vector2i getSelectedChunkTile();

    static inline const sf::Vector2f& getSelectPos() {return selectPos;}
    static inline const sf::Vector2f& getLerpedSelectPos() {return cursorCornerPositions[0].worldPosition;}

    static void setCursorCornersToDestination();

private:
    // Position of each corner in tile cursor
    static std::array<CursorCornerPosition, 4> cursorCornerPositions;
    static std::array<AnimatedTexture, 4> cursorAnimatedTextures;

    // Position of tile cursor
    static sf::Vector2f selectPos;
    static sf::Vector2i selectPosTile;
    static sf::Vector2i selectSize;
};