#pragma once

#include <memory>

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
#include "Entity/HitRect.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"
#include "Entity/EntityBehaviour/EntityBehaviour.hpp"

#include "GUI/InventoryGUI.hpp"
#include "GUI/HitMarkers.hpp"

#include "DebugOptions.hpp"

class Game;
class ChunkManager;

class Entity : public WorldObject
{
public:
    Entity(sf::Vector2f position, EntityType entityType);
    Entity();

    void update(float dt, ProjectileManager& projectileManager, ChunkManager& chunkManager, Game& game, bool onWater, float gameTime);

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const sf::Color& color) const override;
    void createLightSource(LightingEngine& lightingEngine, sf::Vector2f topLeftChunkPos) const override;

    void testHitCollision(const std::vector<HitRect>& hitRects, ChunkManager& chunkManager, float gameTime);
    void damage(int amount, ChunkManager& chunkManager, float gameTime);
    void interact();

    bool isSelectedWithCursor(sf::Vector2f cursorWorldPos);

    void setWorldPosition(sf::Vector2f newPosition);

    EntityType getEntityType();

    sf::Vector2f getSize();

    const CollisionRect& getCollisionRect();
    void setCollisionRect(const CollisionRect& rect);

    sf::Vector2f getVelocity();
    void setVelocity(sf::Vector2f velocity);

    inline bool isAlive() {return health > 0;}

    EntityPOD getPOD(sf::Vector2f chunkPosition);
    void loadFromPOD(const EntityPOD& pod, sf::Vector2f chunkPosition);

private:
    bool isProjectileColliding(Projectile& projectile);

    void initialiseBehaviour(const std::string& behaviour);

private:
    EntityType entityType;
    int health;
    float flash_amount;

    std::unique_ptr<EntityBehaviour> behaviour;

    CollisionRect collisionRect;
    sf::Vector2f velocity;

    AnimatedTextureMinimal idleAnim;
    AnimatedTextureMinimal walkAnim;

};