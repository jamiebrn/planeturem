#pragma once


#include <cmath>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

// #include "Core/SpriteBatch.hpp"
#include "Core/Camera.hpp"
#include "World/ChunkPosition.hpp"
#include "Object/ObjectReference.hpp"
#include "World/LightingEngine.hpp"
#include "GameConstants.hpp"

class Game;

class WorldObject
{
public:
    WorldObject() = default;
    WorldObject(pl::Vector2f position) : position(position) {}

    // General world object functionality

    virtual pl::Vector2f getPosition() const;
    void setPosition(pl::Vector2f pos);

    static ChunkPosition getChunkInside(pl::Vector2f position, int worldSize);
    ChunkPosition getChunkInside(int worldSize) const;

    static pl::Vector2<uint8_t> getChunkTileInside(pl::Vector2f position, int worldSize);

    pl::Vector2<uint8_t> getChunkTileInside(int worldSize) const;

    static ObjectReference getObjectReferenceFromPosition(pl::Vector2f position, int worldSize);
    ObjectReference getThisObjectReference(int worldSize) const;

    static pl::Vector2<uint8_t> getTileInside(pl::Vector2f position);
    pl::Vector2<uint8_t> getTileInside() const;

    static pl::Vector2<uint8_t> getWorldTileInside(pl::Vector2f position, int worldSize);
    pl::Vector2<uint8_t> getWorldTileInside(int worldSize) const;

    // Assumes on water
    static float getWaterBobYOffset(pl::Vector2f position, int worldSize, float gameTime);

    float getWaterBobYOffset(int worldSize, float gameTime) const;

    int getDrawLayer() const;

    // Overridden by inherited classes (specific)
    virtual void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
        const pl::Color& color) const = 0;

    virtual void createLightSource(LightingEngine& lightingEngine, pl::Vector2f topLeftChunkPos, pl::Vector2f playerPos, int worldSize) const {};

protected:
    pl::Vector2f position;
    int drawLayer = 0;
    bool onWater = false;

};