#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>

#include "Core/SpriteBatch.hpp"
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
    WorldObject(sf::Vector2f position) : position(position) {}

    // General world object functionality

    virtual sf::Vector2f getPosition() const;
    void setPosition(sf::Vector2f pos);

    static ChunkPosition getChunkInside(sf::Vector2f position, int worldSize);
    ChunkPosition getChunkInside(int worldSize) const;

    static sf::Vector2i getChunkTileInside(sf::Vector2f position, int worldSize);

    sf::Vector2i getChunkTileInside(int worldSize) const;

    static ObjectReference getObjectReferenceFromPosition(sf::Vector2f position, int worldSize);
    ObjectReference getThisObjectReference(int worldSize) const;

    static sf::Vector2i getTileInside(sf::Vector2f position);
    sf::Vector2i getTileInside() const;

    static sf::Vector2i getWorldTileInside(sf::Vector2f position, int worldSize);
    sf::Vector2i getWorldTileInside(int worldSize) const;

    // Assumes on water
    static float getWaterBobYOffset(sf::Vector2f position, int worldSize, float gameTime);

    float getWaterBobYOffset(int worldSize, float gameTime) const;

    int getDrawLayer() const;

    // Overriden by inherited classes (specific)
    virtual void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
        const sf::Color& color) const = 0;

    virtual void createLightSource(LightingEngine& lightingEngine, sf::Vector2f topLeftChunkPos) const {};

protected:
    sf::Vector2f position;
    int drawLayer = 0;
    bool onWater = false;

};