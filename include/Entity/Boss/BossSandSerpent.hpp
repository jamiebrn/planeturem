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

class BossSandSerpent : public BossEntity
{
public:
    BossSandSerpent(sf::Vector2f playerPosition, Game& game);

    void update(Game& game, ProjectileManager& projectileManager, InventoryData& inventory, Player& player, float dt) override;

    bool isAlive() override;

    void handleWorldWrap(sf::Vector2f positionDelta) override;

    // void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch) override;
    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, float dt, float gameTime, int worldSize, const sf::Color& color) const;

    inline void createLightSource(LightingEngine& lightingEngine, sf::Vector2f topLeftChunkPos) const override {}

    void drawStatsAtCursor(sf::RenderTarget& window, sf::Vector2f mouseScreenPos) override;

    void testCollisionWithPlayer(Player& player) override;

    void getWorldObjects(std::vector<WorldObject*>& worldObjects) override;

private:
    void updateCollision();

    void setPathfindStepIndex(int index);

    void takeDamage(int damage, InventoryData& inventory, sf::Vector2f damagePosition);
    void applyKnockback(Projectile& projectile);

    void giveItemDrops(InventoryData& inventory);

    bool isProjectileColliding(Projectile& projectile);

private:
    enum class BossSandSerpentState
    {
        IdleStage1,
        MovingToPlayer,
        Leaving
    };

private:
    static constexpr int MAX_HEALTH = 1500;
    int health;
    bool dead;

    BossSandSerpentState behaviourState;

    static constexpr int START_MOVE_PLAYER_DISTANCE = 300;
    std::vector<PathfindGridCoordinate> pathfindStepSequence;
    sf::Vector2f pathfindLastStepPosition;
    sf::Vector2f pathfindStepTargetPosition;
    int pathfindStepIndex;

    std::unordered_map<BossSandSerpentState, AnimatedTexture> animations;

    static const std::array<sf::IntRect, 4> HEAD_FRAMES;
    AnimatedTextureMinimal headAnimation;
    // forward = 0, left = -1, right = 1
    int headDirection;
};