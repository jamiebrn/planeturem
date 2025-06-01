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

#include "Network/CompactFloat.hpp"

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
    Projectile(pl::Vector2f position, pl::Vector2f velocity, ProjectileType type, float damageMult, HitLayer hitLayer);

    void update(float dt, int worldSize);

    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const ChunkManager& chunkManager, const Camera& camera);

    int getDamage() const;

    // Called on collision with entity
    void onCollision();

    pl::Vector2f getPosition() const;
    pl::Vector2f getVelocity() const;
    ProjectileType getType() const;

    // void handleWorldWrap(pl::Vector2f positionDelta);

    bool isAlive();

    CollisionCircle getCollisionCircle() const;

    HitLayer getHitLayer() const;

    template <class Archive>
    void save(Archive& ar, const std::uint32_t version) const
    {
        uint8_t projectileTypeCompact = projectileType;

        // Chunk inside
        uint8_t chunkPositionX = std::floor(position.x / (CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED));
        uint8_t chunkPositionY = std::floor(position.y / (CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED));

        // Position in current chunk (uses 22 bits)
        uint32_t positionDataLocal = 0;
        positionDataLocal |= static_cast<uint32_t>(fmod(position.x, CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED) * 10) & 0x7FF;
        positionDataLocal |= (static_cast<uint32_t>(fmod(position.y, CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED) * 10) & 0x7FF) << 11;

        float angle = (std::atan2(velocity.y, velocity.x) + M_PI) / M_PI * 180.0f;

        // Velocity (uses 25 bits)
        uint32_t velocityData = 0;
        velocityData |= static_cast<uint32_t>(angle * 10) & 0xFFF;
        velocityData |= (static_cast<uint32_t>(velocity.getLength() * 10) & 0x1FFF) << 12;

        uint64_t positionVelocityData = chunkPositionX;
        positionVelocityData |= static_cast<uint64_t>(chunkPositionY) << 8;
        positionVelocityData |= static_cast<uint64_t>(positionDataLocal & 0x3FFFFF) << 16;
        positionVelocityData |= static_cast<uint64_t>(velocityData & 0x1FFFFFF) << 38;

        ar(projectileTypeCompact, positionVelocityData);
    }

    template <class Archive>
    void load(Archive& ar, const std::uint32_t version)
    {
        uint8_t projectileTypeCompact;

        uint64_t positionVelocityData;
        
        ar(projectileTypeCompact, positionVelocityData);
        
        projectileType = projectileTypeCompact;

        uint8_t chunkPositionX = positionVelocityData & 0xFF;
        uint8_t chunkPositionY = (positionVelocityData >> 8) & 0xFF;
        uint32_t positionDataLocal = (positionVelocityData >> 16) & 0x3FFFFF;
        uint32_t velocityData = (positionVelocityData >> 38) & 0x1FFFFFF;

        position.x = chunkPositionX * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
        position.x += static_cast<float>(positionDataLocal & 0x7FF) / 10.0f;
        position.y = chunkPositionY * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
        position.y += static_cast<float>((positionDataLocal >> 11) & 0x7FF) / 10.0f;

        float angle = static_cast<float>(velocityData & 0xFFF) / 10.0f / 180.0f * M_PI - M_PI;
        float speed = static_cast<float>((velocityData >> 12) & 0x1FFF) / 10.0f;
        velocity.x = std::cos(angle) * speed;
        velocity.y = std::sin(angle) * speed;
    }

private:
    void initialise(pl::Vector2f position, pl::Vector2f velocity, ProjectileType type, float damageMult, HitLayer hitLayer);

    ProjectileType projectileType;
    int damage;

    pl::Vector2f position;
    pl::Vector2f velocity;
    
    bool alive;
    float timeAlive;

    HitLayer hitLayer;

};

CEREAL_CLASS_VERSION(Projectile, 1);