#pragma once

#include <vector>
#include <memory>

#include <SFML/Graphics.hpp>

#include "Core/SpriteBatch.hpp"

#include "Projectile.hpp"

class ProjectileManager
{
public:
    ProjectileManager() = default;

    void update(float dt);

    void drawProjectiles(sf::RenderTarget& window, SpriteBatch& spriteBatch);

    void addProjectile(std::unique_ptr<Projectile> projectile);

    std::vector<std::unique_ptr<Projectile>>& getProjectiles();

    void handleWorldWrap(sf::Vector2f positionDelta);

private:
    std::vector<std::unique_ptr<Projectile>> projectiles;

};