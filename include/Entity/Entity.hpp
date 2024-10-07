#pragma once

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/SpriteBatch.hpp"

#include "Object/WorldObject.hpp"
#include "World/ChunkManager.hpp"
#include "Data/EntityData.hpp"
#include "Data/EntityDataLoader.hpp"
#include "Data/ItemData.hpp"
#include "Player/InventoryData.hpp"

#include "Entity/EntityPOD.hpp"

#include "GUI/InventoryGUI.hpp"

#include "DebugOptions.hpp"

class ChunkManager;

class Entity : public WorldObject
{
public:
    Entity(sf::Vector2f position, EntityType entityType);
    Entity();

    void update(float dt, ChunkManager& chunkManager, bool onWater);

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) const override;
    void drawLightMask(sf::RenderTarget& lightTexture);

    sf::Vector2f getPositionDrawOffset() const;

    void damage(int amount, InventoryData& inventory);
    void interact();

    bool isSelectedWithCursor(sf::Vector2f cursorWorldPos);

    void setWorldPosition(sf::Vector2f newPosition);

    EntityType getEntityType();

    sf::Vector2f getSize();

    const CollisionRect& getCollisionRect();

    inline bool isAlive() {return health > 0;}

    EntityPOD getPOD(sf::Vector2f chunkPosition);
    void loadFromPOD(const EntityPOD& pod, sf::Vector2f chunkPosition);

private:
    EntityType entityType;
    int health;
    float flash_amount;

    CollisionRect collisionRect;
    sf::Vector2f velocity;

    AnimatedTextureMinimal idleAnim;
    AnimatedTextureMinimal walkAnim;

};