#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <optional>

#include "Core/TextureManager.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/Camera.hpp"
#include "Object/BuildableObject.hpp"
#include "Entity/Entity.hpp"
#include "World/ChunkPosition.hpp"
#include "World/ChunkManager.hpp"
#include "Types/WorldMenuState.hpp"

#include "Data/typedefs.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/ToolData.hpp"
#include "Data/ToolDataLoader.hpp"
#include "GameConstants.hpp"

// #include "GUI/InventoryGUI.hpp"

enum CursorDrawState
{
    Hidden,
    Tile,
    Dynamic
};

struct CursorCornerPosition
{
    sf::Vector2f worldPositionDestination = {0, 0};
    sf::Vector2f worldPosition = {0, 0};
};

// TODO: Probably needs to be rewritten at some point

class Cursor
{
    Cursor() = delete;

public:
    static void updateTileCursor(sf::RenderWindow& window,
                                 float dt,
                                 ChunkManager& chunkManager,
                                 const CollisionRect& playerCollisionRect,
                                 ItemType heldItemType,
                                 ToolType toolType);

    static void updateTileCursorInRoom(sf::RenderWindow& window,
                                       float dt,
                                       const std::vector<std::vector<std::optional<BuildableObject>>>& objectGrid,
                                       ItemType heldItemType,
                                       ToolType toolType);

    static ChunkPosition getSelectedChunk(int worldSize);
    static sf::Vector2i getSelectedChunkTile();
    static sf::Vector2i getSelectedWorldTile(int worldSize);

    static sf::Vector2f getMouseWorldPos(sf::RenderWindow& window);

    static inline const sf::Vector2f& getSelectPos() {return selectPos;}
    static inline const sf::Vector2f& getLerpedSelectPos() {return cursorCornerPositions[0].worldPosition;}

    static void setCursorCornersToDestination();

    static void drawCursor(sf::RenderWindow& window);

    static void setCursorHidden(bool canReach);

    static void handleWorldWrap(sf::Vector2f positionDelta);

private:
    static void updateTileCursorOnPlanetPlaceObject(ObjectType objectType);
    static void updateTileCursorOnPlanetPlaceLand(sf::RenderWindow& window);
    
    static void updateTileCursorOnPlanetToolPickaxe(sf::RenderWindow& window, float dt, ChunkManager& chunkManager, const CollisionRect& playerCollisionRect);
    static void updateTileCursorOnPlanetToolFishingRod(sf::RenderWindow& window, float dt, ChunkManager& chunkManager);

    static void updateTileCursorOnPlanetNoItem(float dt, ChunkManager& chunkManager);
    static void updateTileCursorNoItem(float dt, const std::optional<BuildableObject>& selectedObjectOptional);

    static void updateTileCursorAnimation(float dt);
    
    static void drawTileCursor(sf::RenderWindow& window);

    // Used for drawing cursor when not full tile size, e.g. when selected entity
    static void drawDynamicCursor(sf::RenderWindow& window);

private:
    // Position of each corner in tile cursor
    static std::array<CursorCornerPosition, 4> cursorCornerPositions;
    static std::array<AnimatedTexture, 4> cursorAnimatedTextures;

    static CursorDrawState drawState;

    // Position of tile cursor
    static sf::Vector2f selectPos;
    static sf::Vector2i selectPosTile;
    static sf::Vector2i selectSize;
};