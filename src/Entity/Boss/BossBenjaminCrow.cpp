#include "Entity/Boss/BossBenjaminCrow.hpp"
#include "Game.hpp"

BossBenjaminCrow::BossBenjaminCrow(sf::Vector2f position)
{
    this->position = position;

    idleAnim.create(6, 48, 64, 64, 144, 0.1);

    flyingHeight = 50.0f;
    behaviourState = BossBenjaminState::Idle;

    flashTime = 0.0f;
}

void BossBenjaminCrow::update(Game& game, ProjectileManager& projectileManager, sf::Vector2f playerPos, float dt)
{
    // Update animation
    idleAnim.update(dt);

    flashTime = std::max(flashTime - dt, 0.0f);

    // Update movement
    direction.x = playerPos.x - position.x;
    direction.y = playerPos.y - position.y;
    
    direction = Helper::normaliseVector(direction);

    velocity.x = Helper::lerp(velocity.x, direction.x * MOVE_SPEED, VELOCITY_LERP_WEIGHT * dt);
    velocity.y = Helper::lerp(velocity.y, direction.y * MOVE_SPEED, VELOCITY_LERP_WEIGHT * dt);

    position += velocity * dt;

    // Check projectile collisions
    for (auto& projectile : projectileManager.getProjectiles())
    {
        if (isProjectileColliding(*projectile))
        {
            takeDamage(1);
            applyKnockback(*projectile);
            projectile->onCollision();
        }
    }
}

void BossBenjaminCrow::takeDamage(int damage)
{
    flashTime = MAX_FLASH_TIME;
}

void BossBenjaminCrow::applyKnockback(Projectile& projectile)
{
    sf::Vector2f relativePos = sf::Vector2f(position.x, position.y - flyingHeight) - projectile.getPosition();

    static constexpr float KNOCKBACK_STRENGTH = 20.0f;

    velocity = Helper::normaliseVector(-relativePos) * KNOCKBACK_STRENGTH;
}

bool BossBenjaminCrow::isProjectileColliding(Projectile& projectile)
{
    static constexpr float hitboxSize = 16.0f;

    CollisionCircle collision(position.x, position.y - flyingHeight, hitboxSize);

    sf::Vector2f projectilePos = projectile.getPosition();

    return collision.isPointColliding(projectilePos.x, projectilePos.y);
}

void BossBenjaminCrow::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch)
{
    // Draw shadow
    TextureDrawData drawData;
    drawData.type = TextureType::Entities;

    drawData.position = Camera::worldToScreenTransform(position);

    float scale = ResolutionHandler::getScale();
    drawData.scale = sf::Vector2f(scale, scale);

    drawData.centerRatio = sf::Vector2f(0.5f, 0.5f);

    spriteBatch.draw(window, drawData, sf::IntRect(75, 208, 26, 16));

    // Draw bird
    sf::Vector2f worldPos(position.x, position.y - flyingHeight);
    drawData.position = Camera::worldToScreenTransform(worldPos);

    // Flip if required
    if (direction.x < 0)
    {
        drawData.scale.x *= -1;
    }

    // drawData.centerRatio = sf::Vector2f(0.5f, 1.0f);


    switch (behaviourState)
    {
        case BossBenjaminState::Idle:
        {
            // TODO: Add shader support to spritebatch so I dont have to do this switching
            // If flash required
            if (flashTime > 0)
            {
                spriteBatch.endDrawing(window);

                sf::Shader* flashShader = Shaders::getShader(ShaderType::Flash);
                flashShader->setUniform("flash_amount", flashTime / MAX_FLASH_TIME);

                TextureManager::drawSubTexture(window, drawData, idleAnim.getTextureRect(), flashShader);
            }
            else
            {
                spriteBatch.draw(window, drawData, idleAnim.getTextureRect());
            }
            break;
        }
    }
}

void BossBenjaminCrow::handleWorldWrap(sf::Vector2f positionDelta)
{
    position += positionDelta;
}