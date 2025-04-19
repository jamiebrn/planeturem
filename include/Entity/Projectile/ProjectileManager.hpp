#pragma once

#include <unordered_map>
#include <memory>
#include <cstdint>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/unordered_map.hpp>

#include "Core/Camera.hpp"

#include "Projectile.hpp"

class ChunkManager;

class ProjectileManager
{
public:
    ProjectileManager() = default;

    void update(float dt);

    void drawProjectiles(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const ChunkManager& chunkManager, pl::Vector2f playerPos, const Camera& camera);

    void addProjectile(const Projectile& projectile);
    void createProjectileWithID(uint16_t id, const Projectile& projectile);

    std::unordered_map<uint16_t, Projectile>& getProjectiles();

    void handleWorldWrap(pl::Vector2f positionDelta);

    void clear();

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        ar(projectiles);
    }

private:
    std::unordered_map<uint16_t, Projectile> projectiles;
    uint16_t projectileCounter = 0;

};

CEREAL_CLASS_VERSION(ProjectileManager, 1);