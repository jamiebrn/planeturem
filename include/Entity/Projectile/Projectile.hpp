#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <extlib/cereal/archives/binary.hpp>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Camera.hpp"
#include "Core/Helper.hpp"
#include "Core/CollisionCircle.hpp"

#include "Data/typedefs.hpp"
#include "Data/ToolData.hpp"
#include "Data/ToolDataLoader.hpp"

class ChunkManager;

enum class HitLayer
{
    Player,
    Entity
};

class Projectile
{
public:
    Projectile() = default;
    // Angle in DEGREES
    Projectile(pl::Vector2f position, float angle, ProjectileType type, float damageMult, float shootPower, HitLayer hitLayer);

    void update(float dt);

    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const ChunkManager& chunkManager, pl::Vector2f playerPos, const Camera& camera);

    int getDamage() const;

    // Called on collision with entity
    void onCollision();

    pl::Vector2f getPosition() const;

    void handleWorldWrap(pl::Vector2f positionDelta);

    bool isAlive();

    CollisionCircle getCollisionCircle() const;

    HitLayer getHitLayer() const;

    template <class Archive>
    void save(Archive& ar, const std::uint32_t version) const
    {
        uint8_t projectileTypeCompact = projectileType;
        ar(projectileTypeCompact, position.x, position.y, velocity.x, velocity.y);
    }

    template <class Archive>
    void load(Archive& ar, const std::uint32_t version)
    {
        uint8_t projectileTypeCompact;
        ar(projectileTypeCompact, position.x, position.y, velocity.x, velocity.y);
        projectileType = projectileTypeCompact;
    }

private:
    ProjectileType projectileType;
    int damage;

    pl::Vector2f position;
    pl::Vector2f velocity;
    
    bool alive;
    float timeAlive;

    HitLayer hitLayer;

};

CEREAL_CLASS_VERSION(Projectile, 1);