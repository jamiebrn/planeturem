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

void ProjectileManager::drawProjectiles(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera)
{
    for (auto& projectilePair : projectiles)
    {
        projectilePair.second.draw(window, spriteBatch, camera);
    }
}

void ProjectileManager::addProjectile(const Projectile& projectile)
{
    projectiles[projectileCounter] = projectile;
    projectileCounter++;
}

void ProjectileManager::createProjectileWithID(uint64_t id, const Projectile& projectile)
{
    projectiles[id] = projectile;
}

std::unordered_map<uint64_t, Projectile>& ProjectileManager::getProjectiles()
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