#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>

#include "Core/SpriteBatch.hpp"
#include "World/ChunkPosition.hpp"
#include "World/LightingEngine.hpp"
#include "GameConstants.hpp"

class WorldObject
{
public:
    WorldObject(sf::Vector2f position) : position(position) {}

    // General world object functionality

    sf::Vector2f getPosition() const;
    void setPosition(sf::Vector2f pos);

    static ChunkPosition getChunkInside(sf::Vector2f position, int worldSize);
    ChunkPosition getChunkInside(int worldSize) const;

    static sf::Vector2i getChunkTileInside(sf::Vector2f position, int worldSize);

    sf::Vector2i getChunkTileInside(int worldSize) const;

    static sf::Vector2i getTileInside(sf::Vector2f position);
    sf::Vector2i getTileInside();

    // Assumes on water
    static float getWaterBobYOffset(sf::Vector2f position, int worldSize, float gameTime);

    float getWaterBobYOffset(int worldSize, float gameTime) const;

    int getDrawLayer() const;

    // Overriden by inherited classes (specific)
    virtual void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) const = 0;

    virtual void createLightSource(LightingEngine& lightingEngine, sf::Vector2f topLeftChunkPos) const = 0;

protected:
    sf::Vector2f position;
    int drawLayer = 0;
    bool onWater = false;

};