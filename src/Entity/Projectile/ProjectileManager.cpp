#include "Entity/Projectile/ProjectileManager.hpp"
#include "Game.hpp"
#include "Network/NetworkHandler.hpp"

void ProjectileManager::initialise(Game* game, PlanetType planetType)
{
    this->game = game;
    this->planetType = planetType;
}

void ProjectileManager::update(float dt, int worldSize)
{
    for (auto iter = projectiles.begin(); iter != projectiles.end();)
    {
        Projectile& projectile = *iter;
        
        if (projectile.isAlive())
        {
            projectile.update(dt, worldSize);
        }
        else
        {
            iter = projectiles.erase(iter);
            continue;
        }

        iter++;
    }
}

void ProjectileManager::drawProjectiles(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const ChunkManager& chunkManager,
    pl::Vector2f playerPos, const Camera& camera)
{
    for (const Projectile& projectile : projectiles)
    {
        projectile.draw(window, spriteBatch, chunkManager, camera);
    }
}

void ProjectileManager::addProjectile(const Projectile& projectile, ToolType weaponType)
{
    if (!game)
    {
        printf("ERROR: Projectile manager of planet type %d uninitialised\n", planetType);
        return;
    }

    // Send projectile creation request to host
    if (game->getNetworkHandler().isClient())
    {
        if (weaponType < 0)
        {
            printf("ERROR: Attempted to create networked projectile from null weapon type %d\n", weaponType);
            return;
        }
        
        PacketDataProjectileCreateRequest packetData;
        packetData.planetType = planetType;
        packetData.projectile = projectile;
        packetData.weaponType = weaponType;

        Packet packet;
        packet.set(packetData);

        game->getNetworkHandler().sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
        return;
    }

    projectiles.push_back(projectile);
}

// void ProjectileManager::createProjectileWithID(uint16_t id, const Projectile& projectile)
// {
//     projectiles[id] = projectile;
// }

std::vector<Projectile>& ProjectileManager::getProjectiles()
{
    return projectiles;
}

uint16_t ProjectileManager::getProjectileCount() const
{
    return projectiles.size();
}

// void ProjectileManager::handleWorldWrap(pl::Vector2f positionDelta)
// {
//     for (auto& projectilePair : projectiles)
//     {
//         projectilePair.second.handleWorldWrap(positionDelta);
//     }
// }

void ProjectileManager::clear()
{
    projectiles.clear();
}