#pragma once

#include <unordered_map>
#include <memory>
#include <cstdint>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/unordered_map.hpp>

#include <SFML/Graphics.hpp>

#include "Core/SpriteBatch.hpp"
#include "Core/Camera.hpp"

#include "Projectile.hpp"

class ProjectileManager
{
public:
    ProjectileManager() = default;

    void update(float dt);

    void drawProjectiles(sf::RenderTarget& window, SpriteBatch& spriteBatch, const Camera& camera);

    void addProjectile(const Projectile& projectile);
    void createProjectileWithID(uint64_t id, const Projectile& projectile);

    std::unordered_map<uint64_t, Projectile>& getProjectiles();

    void handleWorldWrap(sf::Vector2f positionDelta);

    void clear();

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        ar(projectiles);
    }

private:
    std::unordered_map<uint64_t, Projectile> projectiles;
    uint64_t projectileCounter = 0;

};

CEREAL_CLASS_VERSION(ProjectileManager, 1);