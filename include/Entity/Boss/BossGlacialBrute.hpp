#pragma once

#include <algorithm>
#include <vector>
#include <array>

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/SpriteBatch.hpp"
#include "Core/TextDraw.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Shaders.hpp"
#include "Core/Camera.hpp"
#include "Core/Tween.hpp"
#include "Core/Helper.hpp"
#include "Core/Sounds.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/CollisionCircle.hpp"

#include "Player/InventoryData.hpp"
#include "Player/Player.hpp"
#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"

#include "World/ChunkManager.hpp"
#include "World/PathfindingEngine.hpp"

#include "GUI/InventoryGUI.hpp"
#include "GUI/HitMarkers.hpp"

#include "Entity/Projectile/Projectile.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"
#include "Entity/HitRect.hpp"

#include "BossEntity.hpp"

class BossGlacialBrute : public BossEntity
{
public:
    BossGlacialBrute(sf::Vector2f playerPosition, Game& game);

    void update(Game& game, ProjectileManager& enemyProjectileManager, Player& player, float dt) override;

    bool isAlive() override;

    void handleWorldWrap(sf::Vector2f positionDelta) override;

    // void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch) override;
    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
        const sf::Color& color) const override;

    inline void createLightSource(LightingEngine& lightingEngine, sf::Vector2f topLeftChunkPos) const override {}

    void getHoverStats(sf::Vector2f mouseWorldPos, std::vector<std::string>& hoverStats) override;

    void testCollisionWithPlayer(Player& player) override;

    void testProjectileCollision(Projectile& projectile, InventoryData& inventory) override;

    void getWorldObjects(std::vector<WorldObject*>& worldObjects) override;

private:
    void updateCollision();

private:
    enum class BossGlacialBruteState
    {
        WalkingToPlayer
    };

private:
    BossGlacialBruteState behaviourState;

    AnimatedTexture walkAnimation;

    static constexpr int MAX_HEALTH = 3500;
    int health;

    static constexpr float MAX_FLASH_TIME = 0.3f;
    float flashTime = 0.0f;

    CollisionRect hitCollision;
};