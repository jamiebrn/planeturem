#pragma once

#include <vector>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/memory.hpp>
#include <extlib/cereal/types/base_class.hpp>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Helper.hpp"

#include "Player/InventoryData.hpp"

#include "Object/WorldObject.hpp"

#include "Entity/Projectile/Projectile.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"
#include "Entity/HitRect.hpp"

class Game;
class Player;
class ChunkManager;
class NetworkHandler;

class BossEntity : public WorldObject
{
public:
    BossEntity() = default;
    virtual BossEntity* clone() const = 0;

    virtual void update(Game& game, ProjectileManager& projectileManager, Player& player, float dt, int worldSize) = 0;

    virtual bool isAlive() = 0;

    // virtual void handleWorldWrap(pl::Vector2f positionDelta) = 0;
    
    virtual void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
        const pl::Color& color) const override = 0;

    virtual inline void createLightSource(LightingEngine& lightingEngine, pl::Vector2f topLeftChunkPos, pl::Vector2f playerPos, int worldSize) const override {};

    virtual void getHoverStats(pl::Vector2f mouseWorldPos, std::vector<std::string>& hoverStats) = 0;

    virtual void testCollisionWithPlayer(Player& player, int worldSize) = 0;

    virtual void testProjectileCollision(Projectile& projectile, int worldSize) {}

    virtual void testHitRectCollision(const std::vector<HitRect>& hitRects, int worldSize) {}

    virtual void getWorldObjects(std::vector<WorldObject*>& worldObjects) = 0;

    // Test for despawn
    bool inPlayerRange(Player& player);

    void createItemPickups(NetworkHandler& networkHandler, ChunkManager& chunkManager, float gameTime);

    void setName(const std::string& name);
    const std::string& getName();

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(position.x, position.y);
    }

protected:
    float playerMaxRange = 1000.0f;
    float itemPickupDropRadius = 16.0f;

    struct ItemDropDistribution
    {
        ItemType itemType = 0;
        int minAmount = 0;
        int maxAmount= 0;
    };

    // Item drops and respective chances
    std::vector<std::pair<ItemDropDistribution, float>> itemDrops;

    std::string name;

};

// CEREAL_REGISTER_TYPE(BossEntity);