#include "Entity/Projectile/ProjectileManager.hpp"

void ProjectileManager::update(float dt)
{
    for (auto iter = projectiles.begin(); iter != projectiles.end();)
    {
        Projectile* projectile = iter->get();
        
        if (projectile->isAlive())
        {
            projectile->update(dt);
        }
        else
        {
            iter = projectiles.erase(iter);
            continue;
        }

        iter++;
    }
}

void ProjectileManager::drawProjectiles(sf::RenderTarget& window, SpriteBatch& spriteBatch, const Camera& camera)
{
    for (auto& projectile : projectiles)
    {
        projectile->draw(window, spriteBatch, camera);
    }
}

void ProjectileManager::addProjectile(std::unique_ptr<Projectile> projectile)
{
    projectiles.push_back(std::move(projectile));
}

std::vector<std::unique_ptr<Projectile>>& ProjectileManager::getProjectiles()
{
    return projectiles;
}

void ProjectileManager::handleWorldWrap(sf::Vector2f positionDelta)
{
    for (auto& projectile : projectiles)
    {
        projectile->handleWorldWrap(positionDelta);
    }
}

void ProjectileManager::clear()
{
    projectiles.clear();
}