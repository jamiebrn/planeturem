#pragma once

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Object/WorldObject.hpp"
#include "World/ChunkManager.hpp"
#include "Data/EntityData.hpp"
#include "Data/EntityDataLoader.hpp"

class ChunkManager;

class Entity : public WorldObject
{
public:
    Entity(sf::Vector2f position, unsigned int entityType);

    void update(float dt, ChunkManager& chunkManager);

    void draw(sf::RenderTarget& window, float dt, const sf::Color& color) override;
    void drawLightMask(sf::RenderTarget& lightTexture);

    void damage(int amount);
    void interact();

    bool isSelectedWithCursor(sf::Vector2f cursorWorldPos);

    unsigned int getEntityType();

    sf::Vector2f getSize();

    inline bool isAlive() {return health > 0;}

private:
    unsigned int entityType;
    int health;
    float flash_amount;

    CollisionRect collisionRect;
    sf::Vector2f velocity;

};