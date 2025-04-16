#pragma once

#include <memory>



#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/AnimatedTexture.hpp"
// #include "Core/SpriteBatch.hpp"

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

#include "Network/PacketData/PacketDataWorld/PacketDataEntities.hpp"

#include "GUI/InventoryGUI.hpp"
#include "GUI/HitMarkers.hpp"

#include "DebugOptions.hpp"

class Game;
class ChunkManager;

class Entity : public WorldObject
{
public:
    Entity(pl::Vector2f position, EntityType entityType);
    Entity();

    void update(float dt, ProjectileManager& projectileManager, ChunkManager& chunkManager, Game& game, bool onWater, float gameTime);

    void updateNetwork(float dt, ChunkManager& chunkManager);

    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const pl::Color& color) const override;
    void createLightSource(LightingEngine& lightingEngine, pl::Vector2f topLeftChunkPos) const override;

    void testHitCollision(const std::vector<HitRect>& hitRects, ChunkManager& chunkManager, Game& game, float gameTime);
    void damage(int amount, ChunkManager& chunkManager, float gameTime);
    void interact();

    bool isSelectedWithCursor(pl::Vector2f cursorWorldPos);

    void setWorldPosition(pl::Vector2f newPosition);

    EntityType getEntityType();

    pl::Vector2f getSize();

    const CollisionRect& getCollisionRect();
    void setCollisionRect(const CollisionRect& rect);

    pl::Vector2f getVelocity();
    void setVelocity(pl::Vector2f velocity);

    void setAnimationSpeed(float speed);

    inline bool isAlive() {return health > 0;}

    EntityPOD getPOD(pl::Vector2f chunkPosition);
    void loadFromPOD(const EntityPOD& pod, pl::Vector2f chunkPosition);

    PacketDataEntities::EntityPacketData getPacketData(pl::Vector2f chunkPosition);
    void loadFromPacketData(const PacketDataEntities::EntityPacketData& packetData, pl::Vector2f chunkPosition);

private:
    bool isProjectileColliding(Projectile& projectile);

    void initialiseBehaviour(const std::string& behaviour);

private:
    EntityType entityType;
    int health;
    float flashAmount;

    std::unique_ptr<EntityBehaviour> behaviour;

    CollisionRect collisionRect;
    CollisionRect hitCollision;
    pl::Vector2f velocity;

    float animationSpeed;

    AnimatedTextureMinimal idleAnim;
    AnimatedTextureMinimal walkAnim;

};