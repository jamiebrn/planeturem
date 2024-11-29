#include "Entity/Boss/BossBenjaminCrow.hpp"
#include "Game.hpp"

const int BossBenjaminCrow::DAMAGE_HITBOX_SIZE = 20;
const std::array<int, 2> BossBenjaminCrow::damageValues = {35, 65};

const std::array<sf::IntRect, 2> BossBenjaminCrow::dashGhostTextureRects = {
    sf::IntRect(400, 160, 48, 64),
    sf::IntRect(400, 224, 48, 64)
};
const sf::IntRect BossBenjaminCrow::deadTextureRect = sf::IntRect(448, 160, 48, 64);
const sf::IntRect BossBenjaminCrow::shadowTextureRect = sf::IntRect(64, 208, 48, 16);

BossBenjaminCrow::BossBenjaminCrow(sf::Vector2f playerPosition)
{
    Sounds::playSound(SoundType::Crow);
    Sounds::playMusic(MusicType::BossTheme1);

    this->position = playerPosition - sf::Vector2f(400, 400);
    drawLayer = -999;

    idleAnims[0].create(6, 48, 64, 112, 160, 0.1);
    idleAnims[1].create(6, 48, 64, 112, 224, 0.1);

    health = MAX_HEALTH;
    dead = false;

    flyingHeight = 50.0f;
    behaviourState = BossBenjaminState::Chase;

    flashTime = 0.0f;

    stage = 0;

    dashCooldownTimer = 0.0f;
    dashGhostTimer = 0.0f;

    updateCollision();
}

void BossBenjaminCrow::update(Game& game, ProjectileManager& projectileManager, ProjectileManager& enemyProjectileManager, InventoryData& inventory, Player& player, float dt)
{
    // Update animation
    idleAnims[stage].update(dt);

    flashTime = std::max(flashTime - dt, 0.0f);

    // If player is dead, fly away
    if (game.getIsDay() || (!player.isAlive() && behaviourState != BossBenjaminState::Killed && behaviourState != BossBenjaminState::Dash))
    {
        behaviourState = BossBenjaminState::FlyAway;
    }

    // Update state
    float playerDistance = Helper::getVectorLength(player.getPosition() - position);

    switch (behaviourState)
    {
        case BossBenjaminState::Chase:
        {
            if (playerDistance >= FAST_CHASE_DISTANCE_THRESHOLD)
            {
                behaviourState = BossBenjaminState::FastChase;
            }
            else if (playerDistance <= DASH_TARGET_INITIATE_DISTANCE && dashCooldownTimer <= 0)
            {
                behaviourState = BossBenjaminState::Dash;
                dashTargetPosition = Helper::normaliseVector(player.getPosition() - (position - sf::Vector2f(0, flyingHeight))) * DASH_TARGET_OVERSHOOT + player.getPosition();
            }
            break;
        }
        case BossBenjaminState::FastChase:
        {
            if (playerDistance < FAST_CHASE_DISTANCE_THRESHOLD)
            {
                behaviourState = BossBenjaminState::Chase;
            }
            break;
        }
        case BossBenjaminState::Dash:
        {
            float distanceToTarget = Helper::getVectorLength(dashTargetPosition - (position - sf::Vector2f(0, flyingHeight)));
            if (distanceToTarget <= DASH_TARGET_FINISH_DISTANCE)
            {
                if (stage == 0)
                {
                    dashCooldownTimer = MAX_DASH_COOLDOWN_TIMER;
                }
                else
                {
                    dashCooldownTimer = SECOND_STAGE_MAX_DASH_COOLDOWN_TIMER;
                }
                behaviourState = BossBenjaminState::Chase;
            }
            break;
        }
        case BossBenjaminState::FlyAway:
        {
            if (!game.getIsDay() && player.isAlive())
            {
                behaviourState = BossBenjaminState::Chase;
            }
            break;
        }
    }

    // Update movement based on state

    switch (behaviourState)
    {
        case BossBenjaminState::Chase:
        {
            direction = player.getPosition() - position;
            currentMoveSpeed = MOVE_SPEED;
            break;
        }
        case BossBenjaminState::FastChase:
        {
            direction = player.getPosition() - position;
            currentMoveSpeed = FAST_CHASE_MOVE_SPEED;
            break;
        }
        case BossBenjaminState::Dash:
        {
            direction = dashTargetPosition - (position - sf::Vector2f(0, flyingHeight));
            currentMoveSpeed = DASH_MOVE_SPEED;

            // Dash effect
            dashGhostTimer += dt;
            if (dashGhostTimer >= MAX_DASH_GHOST_TIMER)
            {
                dashGhostTimer = 0;
                addDashGhostEffect();
            }
            break;
        }
        case BossBenjaminState::FlyAway:
        {
            direction = position - player.getPosition();
            currentMoveSpeed = FAST_CHASE_MOVE_SPEED;
            break;
        }
    }

    if (behaviourState != BossBenjaminState::Killed)
    {    
        direction = Helper::normaliseVector(direction);

        float speedMult = 1.0f;
        // Add speed mult on second stage
        if (stage == 1)
        {
            speedMult = SECOND_STAGE_SPEED_MULTIPLIER;
        }

        velocity.x = Helper::lerp(velocity.x, direction.x * currentMoveSpeed * speedMult, VELOCITY_LERP_WEIGHT * dt);
        velocity.y = Helper::lerp(velocity.y, direction.y * currentMoveSpeed * speedMult, VELOCITY_LERP_WEIGHT * dt);

        position += velocity * dt;
        
        // Check projectile collisions   
        for (auto& projectile : projectileManager.getProjectiles())
        {
            if (isProjectileColliding(*projectile))
            {
                takeDamage(projectile->getDamage(), inventory, projectile->getPosition());
                applyKnockback(*projectile);
                projectile->onCollision();
            }
        }
    }
    else
    {
        floatTween.update(dt);

        // If killed, check falling tween complete
        if (floatTween.isTweenFinished(fallingTweenID))
        {
            dead = true;
        }
    }

    // Update collision
    updateCollision();
    testCollisionWithPlayer(player);

    // Update dash cooldown timer
    dashCooldownTimer -= dt;

    // Update dash ghost effects
    updateDashGhostEffects(dt);
}

void BossBenjaminCrow::updateCollision()
{
    collision = CollisionCircle(position.x, position.y - flyingHeight, HITBOX_SIZE);
}

void BossBenjaminCrow::takeDamage(int damage, InventoryData& inventory, sf::Vector2f damagePosition)
{
    flashTime = MAX_FLASH_TIME;

    health -= damage;
    HitMarkers::addHitMarker(damagePosition, damage);

    if (health <= HEALTH_SECOND_STAGE_THRESHOLD)
    {
        stage = 1;
    }

    if (health <= 0)
    {
        // TODO: Manage item drops in boss manager on boss removal
        giveItemDrops(inventory);
        behaviourState = BossBenjaminState::Killed;

        // Start falling tween
        fallingTweenID = floatTween.startTween(&flyingHeight, flyingHeight, 0.0f, TWEEN_DEAD_FALLING_TIME, TweenTransition::Sine, TweenEasing::EaseIn);
    }
}

void BossBenjaminCrow::applyKnockback(Projectile& projectile)
{
    if (behaviourState == BossBenjaminState::Dash)
    {
        return;
    }

    sf::Vector2f relativePos = sf::Vector2f(position.x, position.y - flyingHeight) - projectile.getPosition();

    static constexpr float KNOCKBACK_STRENGTH = 7.0f;

    velocity -= Helper::normaliseVector(-relativePos) * KNOCKBACK_STRENGTH;
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

void BossBenjaminCrow::addDashGhostEffect()
{
    DashGhostEffect ghostEffect;
    ghostEffect.timer = DashGhostEffect::MAX_TIME;
    ghostEffect.position = position - sf::Vector2f(0, flyingHeight);
    ghostEffect.stage = stage;

    if (direction.x < 0)
    {
        ghostEffect.scaleX = -1;
    }

    dashGhostEffects.push_back(ghostEffect);
}

void BossBenjaminCrow::updateDashGhostEffects(float dt)
{
    for (auto iter = dashGhostEffects.begin(); iter != dashGhostEffects.end();)
    {
        iter->timer -= dt;
        if (iter->timer <= 0)
        {
            iter = dashGhostEffects.erase(iter);
            continue;
        }
        iter++;
    }
}

bool BossBenjaminCrow::isAlive()
{
    if (dead)
    {
        // Unlock achievement
        Achievements::attemptAchievementUnlock("KILLED_BENJAMIN");
    }

    return (!dead);
}

void BossBenjaminCrow::handleWorldWrap(sf::Vector2f positionDelta)
{
    position += positionDelta;
    dashTargetPosition += positionDelta;

    // Wrap dash ghost effects
    for (DashGhostEffect& dashGhostEffect : dashGhostEffects)
    {
        dashGhostEffect.position += positionDelta;
    }
}

void BossBenjaminCrow::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const sf::Color& color) const
{
    // Draw shadow
    TextureDrawData drawData;
    drawData.type = TextureType::Entities;

    drawData.position = camera.worldToScreenTransform(position);

    float scale = ResolutionHandler::getScale();
    drawData.scale = sf::Vector2f(scale, scale);

    drawData.centerRatio = sf::Vector2f(0.5f, 0.5f);

    spriteBatch.draw(window, drawData, shadowTextureRect);

    // Draw ghost effects
    for (const DashGhostEffect& dashGhostEffect : dashGhostEffects)
    {
        TextureDrawData effectDrawData;
        effectDrawData.type = TextureType::Entities;

        effectDrawData.position = camera.worldToScreenTransform(dashGhostEffect.position);
        effectDrawData.colour = sf::Color(255, 255, 255, dashGhostEffect.MAX_ALPHA * dashGhostEffect.timer / dashGhostEffect.MAX_TIME);
        effectDrawData.scale = sf::Vector2f(scale * dashGhostEffect.scaleX, scale);
        effectDrawData.centerRatio = sf::Vector2f(0.5f, 0.5f);

        spriteBatch.draw(window, effectDrawData, dashGhostTextureRects[dashGhostEffect.stage]);
    }

    // Draw bird
    sf::Vector2f worldPos(position.x, position.y - flyingHeight);
    drawData.position = camera.worldToScreenTransform(worldPos);

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
        case BossBenjaminState::Chase:
        {        
            spriteBatch.draw(window, drawData, idleAnims[stage].getTextureRect(), shaderType);
            break;
        }
        case BossBenjaminState::FastChase: // fallthrough
        case BossBenjaminState::FlyAway: // fallthrough
        case BossBenjaminState::Dash:
        {        
            spriteBatch.draw(window, drawData, dashGhostTextureRects[stage], shaderType);
            break;
        }
        case BossBenjaminState::Killed:
        {
            spriteBatch.draw(window, drawData, deadTextureRect, shaderType);
            break;
        }
    }
}

void BossBenjaminCrow::getHoverStats(sf::Vector2f mouseWorldPos, std::vector<std::string>& hoverStats)
{
    if (!collision.isPointColliding(mouseWorldPos.x, mouseWorldPos.y))
    {
        return;
    }

    hoverStats.push_back("Benjamin (" + std::to_string(health) + " / " + std::to_string(MAX_HEALTH) + ")");
}

void BossBenjaminCrow::testCollisionWithPlayer(Player& player)
{
    if (behaviourState == BossBenjaminState::Killed)
    {
        return;
    }

    HitRect hitRect;
    hitRect.x = position.x - DAMAGE_HITBOX_SIZE;
    hitRect.y = position.y - flyingHeight - DAMAGE_HITBOX_SIZE;
    hitRect.height = DAMAGE_HITBOX_SIZE * 2;
    hitRect.width = DAMAGE_HITBOX_SIZE * 2;

    hitRect.damage = damageValues[stage];

    player.testHitCollision(hitRect);
}

void BossBenjaminCrow::getWorldObjects(std::vector<WorldObject*>& worldObjects)
{
    worldObjects.push_back(this);
}