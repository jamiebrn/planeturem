#include "Entity/Boss/BossBenjaminCrow.hpp"
#include "Game.hpp"

BossBenjaminCrow::BossBenjaminCrow(sf::Vector2f position)
{
    this->position = position;

    idleAnim.create(6, 48, 64, 64, 144, 0.1);

    health = MAX_HEALTH;

    flyingHeight = 50.0f;
    behaviourState = BossBenjaminState::Idle;

    flashTime = 0.0f;

    updateCollision();    
}

void BossBenjaminCrow::update(Game& game, ProjectileManager& projectileManager, InventoryData& inventory, sf::Vector2f playerPos, float dt)
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
            takeDamage(1, inventory);
            applyKnockback(*projectile);
            projectile->onCollision();
        }
    }

    // Update collision
    updateCollision();
}

void BossBenjaminCrow::updateCollision()
{
    collision = CollisionCircle(position.x, position.y - flyingHeight, hitboxSize);
}

void BossBenjaminCrow::takeDamage(int damage, InventoryData& inventory)
{
    flashTime = MAX_FLASH_TIME;

    health -= damage;

    if (health <= 0)
    {
        giveItemDrops(inventory);
    }
}

void BossBenjaminCrow::applyKnockback(Projectile& projectile)
{
    sf::Vector2f relativePos = sf::Vector2f(position.x, position.y - flyingHeight) - projectile.getPosition();

    static constexpr float KNOCKBACK_STRENGTH = 20.0f;

    velocity = Helper::normaliseVector(-relativePos) * KNOCKBACK_STRENGTH;
}

void BossBenjaminCrow::giveItemDrops(InventoryData& inventory)
{
    static const std::vector<ItemCount> itemDrops = {
        {ItemDataLoader::getItemTypeFromName("Feather"), 10},
        {ItemDataLoader::getItemTypeFromName("Bone"), 5},
        {ItemDataLoader::getItemTypeFromName("Crow Claw"), 2},
        {ItemDataLoader::getItemTypeFromName("Crow Skull"), 1}
    };

    for (const ItemCount& itemDrop : itemDrops)
    {
        inventory.addItem(itemDrop.first, itemDrop.second);
        InventoryGUI::pushItemPopup(itemDrop);
    }
}

bool BossBenjaminCrow::isProjectileColliding(Projectile& projectile)
{
    sf::Vector2f projectilePos = projectile.getPosition();

    return collision.isPointColliding(projectilePos.x, projectilePos.y);
}

bool BossBenjaminCrow::isAlive()
{
    return (health > 0);
}

void BossBenjaminCrow::handleWorldWrap(sf::Vector2f positionDelta)
{
    position += positionDelta;
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

    // Apply flash if required
    std::optional<ShaderType> shaderType;

    if (flashTime > 0)
    {
        shaderType = ShaderType::Flash;
        sf::Shader* shader = Shaders::getShader(shaderType.value());
        shader->setUniform("flash_amount", flashTime / MAX_FLASH_TIME);
    }

    switch (behaviourState)
    {
        case BossBenjaminState::Idle:
        {        
            spriteBatch.draw(window, drawData, idleAnim.getTextureRect(), shaderType);
            break;
        }
    }
}

void BossBenjaminCrow::drawStatsAtCursor(sf::RenderTarget& window, sf::Vector2f mouseScreenPos)
{
    sf::Vector2f mouseWorldPos = Camera::screenToWorldTransform(mouseScreenPos);

    if (!collision.isPointColliding(mouseWorldPos.x, mouseWorldPos.y))
    {
        return;
    }

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    TextDrawData textDrawData;
    textDrawData.text = "Benjamin (" + std::to_string(health) + " / " + std::to_string(MAX_HEALTH) + ")";
    textDrawData.position = mouseScreenPos + sf::Vector2f(STATS_DRAW_OFFSET_X * intScale, STATS_DRAW_OFFSET_Y * intScale);
    textDrawData.colour = sf::Color(255, 255, 255, 255);
    textDrawData.size = STATS_DRAW_SIZE;
    textDrawData.containOnScreenX = true;
    textDrawData.containOnScreenY = true;

    TextDraw::drawText(window, textDrawData);
}