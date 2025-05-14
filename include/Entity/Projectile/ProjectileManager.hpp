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

#include "Network/Packet.hpp"
#include "Network/PacketData/PacketDataWorld/PacketDataProjectileCreateRequest.hpp"

#include "Core/Camera.hpp"

#include "Projectile.hpp"

class Game;
class ChunkManager;

class ProjectileManager
{
public:
    ProjectileManager() = default;
    void initialise(Game* game, PlanetType planetType);

    void update(float dt);

    void drawProjectiles(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const ChunkManager& chunkManager, pl::Vector2f playerPos, const Camera& camera);

    // Will request projectile from host if is client
    void addProjectile(const Projectile& projectile, ToolType weaponType = -1);

    void createProjectileWithID(uint16_t id, const Projectile& projectile);

    std::unordered_map<uint16_t, Projectile>& getProjectiles();

    uint16_t getProjectileCount() const;

    // void handleWorldWrap(pl::Vector2f positionDelta);

    void clear();

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        ar(projectiles);
    }

private:
    std::unordered_map<uint16_t, Projectile> projectiles;
    uint16_t projectileCounter = 0;

    Game* game;
    PlanetType planetType;

};

CEREAL_CLASS_VERSION(ProjectileManager, 1);