#pragma once

#include <vector>

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

class BossEntity : public WorldObject
{
public:
    BossEntity() = default;

    virtual void update(Game& game, ProjectileManager& projectileManager, Player& player, float dt) = 0;

    virtual bool isAlive() = 0;

    virtual void handleWorldWrap(sf::Vector2f positionDelta) = 0;

    // virtual void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch) = 0;
    virtual void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
        const sf::Color& color) const override = 0;

    virtual void createLightSource(LightingEngine& lightingEngine, sf::Vector2f topLeftChunkPos) const override = 0;

    virtual void getHoverStats(sf::Vector2f mouseWorldPos, std::vector<std::string>& hoverStats) = 0;

    virtual void testCollisionWithPlayer(Player& player) = 0;

    virtual void testProjectileCollision(Projectile& projectile) {}

    virtual void testHitRectCollision(const std::vector<HitRect>& hitRects) {}

    virtual void getWorldObjects(std::vector<WorldObject*>& worldObjects) = 0;

    // Test for despawn
    bool inPlayerRange(Player& player);

    void createItemPickups(ChunkManager& chunkManager, float gameTime);

    void setName(const std::string& name);
    const std::string& getName();

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