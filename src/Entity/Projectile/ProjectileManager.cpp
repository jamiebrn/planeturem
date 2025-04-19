#include "Entity/Projectile/ProjectileManager.hpp"

void ProjectileManager::update(float dt)
{
    for (auto iter = projectiles.begin(); iter != projectiles.end();)
    {
        Projectile& projectile = iter->second;
        
        if (projectile.isAlive())
        {
            projectile.update(dt);
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
    for (auto& projectilePair : projectiles)
    {
        projectilePair.second.draw(window, spriteBatch, chunkManager, playerPos, camera);
    }
}

void ProjectileManager::addProjectile(const Projectile& projectile)
{
    if (projectiles.contains(projectileCounter))
    {
        printf("ERROR: Failed to create projectile with already existing ID %d\n", projectileCounter);
        return;
    }

    projectiles[projectileCounter] = projectile;
    projectileCounter++;
}

void ProjectileManager::createProjectileWithID(uint16_t id, const Projectile& projectile)
{
    projectiles[id] = projectile;
}

std::unordered_map<uint16_t, Projectile>& ProjectileManager::getProjectiles()
{
    return projectiles;
}

void ProjectileManager::handleWorldWrap(pl::Vector2f positionDelta)
{
    for (auto& projectilePair : projectiles)
    {
        projectilePair.second.handleWorldWrap(positionDelta);
    }
}

void ProjectileManager::clear()
{
    projectiles.clear();
}