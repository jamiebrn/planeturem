#pragma once

#include <array>
#include <vector>
#include <optional>
#include <memory>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/Camera.hpp"
#include "Object/BuildableObject.hpp"
#include "Entity/Entity.hpp"
#include "World/ChunkPosition.hpp"
#include "World/ChunkManager.hpp"
#include "World/Room.hpp"
#include "Types/WorldMenuState.hpp"

#include "Player/InventoryData.hpp"

#include "GUI/InventoryGUI.hpp"

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
    Tile
    // Dynamic
};

struct CursorCornerPosition
{
    pl::Vector2f worldPositionDestination = {0, 0};
    pl::Vector2f worldPosition = {0, 0};
};

// TODO: Probably needs to be rewritten at some point

class Cursor
{
    Cursor() = delete;

public:
    static void updateTileCursor(pl::Vector2f mouseWorldPos,
                                 float dt,
                                 ChunkManager& chunkManager,
                                 const CollisionRect& playerCollisionRect,
                                 InventoryData& inventory,
                                 WorldMenuState worldMenuState);

    static void updateTileCursorInRoom(pl::Vector2f mouseWorldPos,
                                       float dt,
                                       const Room& room,
                                       ItemType heldItemType,
                                       ToolType toolType);

    static ChunkPosition getSelectedChunk(int worldSize);
    static pl::Vector2<int> getSelectedChunkTile();
    static pl::Vector2<int> getSelectedWorldTile(int worldSize);
    static pl::Vector2<int> getSelectedTile();

    // static pl::Vector2f getMouseWorldPos(pl::RenderTarget& window, const Camera& camera);

    static inline const pl::Vector2f& getSelectPos() {return selectPos;}
    static inline const pl::Vector2f& getLerpedSelectPos() {return cursorCornerPositions[0].worldPosition;}

    static void setCursorCornersToDestination();

    static void drawCursor(pl::RenderTarget& window, const Camera& camera, int worldSize);

    static void setCursorHidden(bool canReach);

    // static void handleWorldWrap(pl::Vector2f positionDelta);

private:
    static void updateTileCursorOnPlanetPlaceObject(ObjectType objectType);
    static void updateTileCursorOnPlanetPlaceLand();
    
    static void updateTileCursorOnPlanetToolPickaxe(pl::Vector2f mouseWorldPos, float dt, ChunkManager& chunkManager, const CollisionRect& playerCollisionRect);
    static void updateTileCursorOnPlanetToolFishingRod(float dt, ChunkManager& chunkManager);

    static void updateTileCursorOnPlanetNoItem(float dt, ChunkManager& chunkManager);
    static void updateTileCursorNoItem(float dt, BuildableObject* selectedObject);

    static void updateTileCursorAnimation(float dt);
    
    static void drawTileCursor(pl::RenderTarget& window, const Camera& camera, int worldSize);

    // Used for drawing cursor when not full tile size, e.g. when selected entity
    // static void drawDynamicCursor(pl::RenderTarget& window, const Camera& camera);

private:
    // Position of each corner in tile cursor
    static std::array<CursorCornerPosition, 4> cursorCornerPositions;
    static std::array<AnimatedTexture, 4> cursorAnimatedTextures;

    static CursorDrawState drawState;

    // Position of tile cursor
    static pl::Vector2f selectPos;
    static pl::Vector2<int> selectPosTile;
    static pl::Vector2<int> selectSize;

};