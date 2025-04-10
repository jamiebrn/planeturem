#include "Entity/Boss/BossBenjaminCrow.hpp"
#include "Game.hpp"

const int BossBenjaminCrow::DAMAGE_HITBOX_SIZE = 20;
const std::array<int, 2> BossBenjaminCrow::damageValues = {35, 65};

const std::array<pl::Rect<int>, 2> BossBenjaminCrow::dashGhostTextureRects = {
    pl::Rect<int>(400, 160, 48, 64),
    pl::Rect<int>(400, 224, 48, 64)
};
const pl::Rect<int> BossBenjaminCrow::deadTextureRect = pl::Rect<int>(448, 160, 48, 64);
const pl::Rect<int> BossBenjaminCrow::shadowTextureRect = pl::Rect<int>(64, 208, 48, 16);

BossBenjaminCrow::BossBenjaminCrow(pl::Vector2f playerPosition)
{
    Sounds::playSound(SoundType::Crow);
    Sounds::playMusic(MusicType::BossTheme1);

    itemDrops = {
        {{ItemDataLoader::getItemTypeFromName("Feather"), 7, 12}, 1.0},
        {{ItemDataLoader::getItemTypeFromName("Bone"), 3, 7}, 1.0},
        {{ItemDataLoader::getItemTypeFromName("Crow Claw"), 1, 2}, 0.5},
        {{ItemDataLoader::getItemTypeFromName("Crow Skull"), 1, 1}, 0.4}
    };

    this->position = playerPosition - pl::Vector2f(400, 400);
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

void BossBenjaminCrow::update(Game& game, ProjectileManager& enemyProjectileManager, Player& player, float dt)
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
                dashTargetPosition = Helper::normaliseVector(player.getPosition() - (position - pl::Vector2f(0, flyingHeight))) * DASH_TARGET_OVERSHOOT + player.getPosition();
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
            float distanceToTarget = Helper::getVectorLength(dashTargetPosition - (position - pl::Vector2f(0, flyingHeight)));
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
            direction = dashTargetPosition - (position - pl::Vector2f(0, flyingHeight));
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

void BossBenjaminCrow::takeDamage(int damage, pl::Vector2f damagePosition)
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

    pl::Vector2f relativePos = pl::Vector2f(position.x, position.y - flyingHeight) - projectile.getPosition();

    static constexpr float KNOCKBACK_STRENGTH = 7.0f;

    velocity -= Helper::normaliseVector(-relativePos) * KNOCKBACK_STRENGTH;
}

bool BossBenjaminCrow::isProjectileColliding(Projectile& projectile)
{
    return collision.isColliding(projectile.getCollisionCircle());
}

void BossBenjaminCrow::addDashGhostEffect()
{
    DashGhostEffect ghostEffect;
    ghostEffect.timer = DashGhostEffect::MAX_TIME;
    ghostEffect.position = position - pl::Vector2f(0, flyingHeight);
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

void BossBenjaminCrow::handleWorldWrap(pl::Vector2f positionDelta)
{
    position += positionDelta;
    dashTargetPosition += positionDelta;

    // Wrap dash ghost effects
    for (DashGhostEffect& dashGhostEffect : dashGhostEffects)
    {
        dashGhostEffect.position += positionDelta;
    }
}

void BossBenjaminCrow::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const pl::Color& color) const
{
    // Draw shadow
    pl::DrawData drawData;
    drawData.texture = TextureManager::getTexture(TextureType::Entities);
    drawData.shader = Shaders::getShader(ShaderType::Default);

    drawData.position = camera.worldToScreenTransform(position);

    float scale = ResolutionHandler::getScale();
    drawData.scale = pl::Vector2f(scale, scale);

    drawData.centerRatio = pl::Vector2f(0.5f, 0.5f);
    drawData.textureRect = shadowTextureRect;

    spriteBatch.draw(window, drawData);

    // Draw ghost effects
    for (const DashGhostEffect& dashGhostEffect : dashGhostEffects)
    {
        pl::DrawData effectDrawData;
        effectDrawData.texture = TextureManager::getTexture(TextureType::Entities);
        effectDrawData.shader = Shaders::getShader(ShaderType::Default);

        effectDrawData.position = camera.worldToScreenTransform(dashGhostEffect.position);
        effectDrawData.color = pl::Color(255, 255, 255, dashGhostEffect.MAX_ALPHA * dashGhostEffect.timer / dashGhostEffect.MAX_TIME);
        effectDrawData.scale = pl::Vector2f(scale * dashGhostEffect.scaleX, scale);
        effectDrawData.centerRatio = pl::Vector2f(0.5f, 0.5f);
        effectDrawData.textureRect = dashGhostTextureRects[dashGhostEffect.stage];

        spriteBatch.draw(window, effectDrawData);
    }

    // Draw bird
    pl::Vector2f worldPos(position.x, position.y - flyingHeight);
    drawData.position = camera.worldToScreenTransform(worldPos);

    // Flip if required
    if (direction.x < 0)
    {
        drawData.scale.x *= -1;
    }

    // drawData.centerRatio = pl::Vector2f(0.5f, 1.0f);

    // Apply flash if required
    if (flashTime > 0)
    {
        drawData.shader = Shaders::getShader(ShaderType::Flash);
        drawData.shader->setUniform1f("flash_amount", flashTime / MAX_FLASH_TIME);
    }

    switch (behaviourState)
    {
        case BossBenjaminState::Chase:
        {
            drawData.textureRect = idleAnims[stage].getTextureRect();
            break;
        }
        case BossBenjaminState::FastChase: // fallthrough
        case BossBenjaminState::FlyAway: // fallthrough
        case BossBenjaminState::Dash:
        {        
            drawData.textureRect = dashGhostTextureRects[stage];
            break;
        }
        case BossBenjaminState::Killed:
        {
            drawData.textureRect = deadTextureRect;
            break;
        }
    }

    spriteBatch.draw(window, drawData);
}

void BossBenjaminCrow::getHoverStats(pl::Vector2f mouseWorldPos, std::vector<std::string>& hoverStats)
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

void BossBenjaminCrow::testProjectileCollision(Projectile& projectile)
{
    if (behaviourState == BossBenjaminState::Killed)
    {    
        return;
    }
    
    if (isProjectileColliding(projectile))
    {
        takeDamage(projectile.getDamage(), projectile.getPosition());
        applyKnockback(projectile);
        projectile.onCollision();
    }
}

void BossBenjaminCrow::getWorldObjects(std::vector<WorldObject*>& worldObjects)
{
    worldObjects.push_back(this);
}