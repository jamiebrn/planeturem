#pragma once

#include <vector>
#include <memory>
#include <cstdint>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>

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

    void update(float dt, int worldSize);

    void drawProjectiles(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const ChunkManager& chunkManager, pl::Vector2f playerPos, const Camera& camera);

    // Will request projectile from host if is client
    void addProjectile(const Projectile& projectile, ToolType weaponType = -1);

    // void createProjectileWithID(uint16_t id, const Projectile& projectile);

    std::vector<Projectile>& getProjectiles();

    uint16_t getProjectileCount() const;

    // void handleWorldWrap(pl::Vector2f positionDelta);

    void clear();

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        ar(projectiles);
    }

private:
    std::vector<Projectile> projectiles;

    Game* game;
    PlanetType planetType;

};

CEREAL_CLASS_VERSION(ProjectileManager, 1);