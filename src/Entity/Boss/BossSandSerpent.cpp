#include "Entity/Boss/BossSandSerpent.hpp"
#include "Game.hpp"

const std::array<pl::Rect<int>, 4> BossSandSerpent::HEAD_FRAMES = {
    pl::Rect<int>(0, 384, 48, 32),
    pl::Rect<int>(48, 384, 48, 32),
    pl::Rect<int>(96, 384, 48, 32),
    pl::Rect<int>(144, 384, 48, 32),
};

const pl::Rect<int> BossSandSerpent::SHOOTING_HEAD_FRAME = pl::Rect<int>(192, 416, 48, 32);

BossSandSerpent::BossSandSerpent(pl::Vector2f playerPosition, Game& game)
{
    // Sounds::playSound(SoundType::Crow);
    Sounds::playMusic(MusicType::BossTheme1);

    itemDrops = {
        {{ItemDataLoader::getItemTypeFromName("Serpent Venom"), 55, 90}, 1.0},
        {{ItemDataLoader::getItemTypeFromName("Sandscale"), 2, 5}, 1.0},
        {{ItemDataLoader::getItemTypeFromName("Serpent Tongue"), 1, 1}, 0.4},
        {{ItemDataLoader::getItemTypeFromName("Serpent Mask"), 1, 1}, 0.2},
        {{ItemDataLoader::getItemTypeFromName("Serpent Sceptre"), 1, 1}, 0.1}
    };

    pl::Vector2<int> playerTile = getWorldTileInside(playerPosition, game.getChunkManager().getWorldSize());

    PathfindGridCoordinate spawnTileRelative = game.getChunkManager().getPathfindingEngine().findFurthestOpenTile(playerTile.x, playerTile.y, 40, true);

    position = pl::Vector2f(playerTile.x + spawnTileRelative.x + 0.5f, playerTile.y + spawnTileRelative.y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;

    animations[BossSandSerpentState::IdleStage1] = AnimatedTexture(4, 80, 51, 0, 301, 0.1);
    animations[BossSandSerpentState::ShootingStage1] = AnimatedTexture(3, 80, 51, 0, 301, 0.1, false);
    animations[BossSandSerpentState::MovingToPlayer] = AnimatedTexture(4, 16, 32, 192, 384, 0.1);
    animations[BossSandSerpentState::Leaving] = animations[BossSandSerpentState::MovingToPlayer];

    headHealth = MAX_HEAD_HEALTH;
    bodyHealth = MAX_BODY_HEALTH;
    dead = false;

    headFlashTime = 0.0f;
    bodyFlashTime = 0.0f;

    shootCooldownTime = 0.0f;
    shootProjectileCooldownTime = 0.0f;
    idleCooldownTime = 0.0f;

    behaviourState = BossSandSerpentState::IdleStage1;

    updateCollision();
}

BossEntity* BossSandSerpent::clone() const
{
    return new BossSandSerpent(*this);
}

void BossSandSerpent::update(Game& game, ProjectileManager& projectileManager, std::vector<Player*>& players, float dt, int worldSize)
{
    if (game.getNetworkHandler().isClient())
    {
        return;
    }

    bool playerAlive = isPlayerAlive(players);
    Player* closestPlayer = findClosestPlayer(players, worldSize);

    if (!closestPlayer)
    {
        return;
    }

    pl::Vector2f closestPlayerRelativePos = Camera::translateWorldPos(closestPlayer->getPosition(), position, worldSize);

    switch (behaviourState)
    {
        case BossSandSerpentState::IdleStage1:
        {
            // Update shooting
            idleCooldownTime += dt;
            if (idleCooldownTime >= MAX_IDLE_COOLDOWN_TIME && headHealth > 0)
            {
                idleCooldownTime = 0.0f;
                shootProjectileCooldownTime = 0.0f;
                behaviourState = BossSandSerpentState::ShootingStage1;
            }

            float playerDistance = Helper::getVectorLength(closestPlayerRelativePos - position);
            if (playerDistance >= START_MOVE_PLAYER_DISTANCE)
            {
                // Find path to player and change behaviour state to move towards
                const PathfindingEngine& pathfindingEngine = game.getChunkManager().getPathfindingEngine();

                int worldSize = game.getChunkManager().getWorldSize();

                pl::Vector2<int> tile = getWorldTileInside(worldSize);
                pl::Vector2<int> playerTile = closestPlayer->getWorldTileInside(worldSize);

                std::vector<PathfindGridCoordinate> pathfindResult;
                if (pathfindingEngine.findPath(tile.x, tile.y, playerTile.x, playerTile.y, pathfindResult, false, 50))
                {
                    pathFollower.beginPath(position, pathfindingEngine.createStepSequenceFromPath(pathfindResult));

                    behaviourState = BossSandSerpentState::MovingToPlayer;
                }
                else
                {
                    behaviourState = BossSandSerpentState::Leaving;
                }
            }
            break;
        }
        case BossSandSerpentState::ShootingStage1:
        {
            shootCooldownTime += dt;
            if (shootCooldownTime >= MAX_SHOOT_COOLDOWN_TIME || headHealth <= 0)
            {
                shootCooldownTime = 0.0f;
                behaviourState = BossSandSerpentState::IdleStage1;
                break;
            }

            shootProjectileCooldownTime += dt;
            if (shootProjectileCooldownTime >= MAX_SHOOT_PROJECTILE_COOLDOWN_TIME)
            {
                shootProjectileCooldownTime = 0.0f;
                float angle = std::atan2(closestPlayerRelativePos.y - 4 - (position.y - 50), closestPlayerRelativePos.x - position.x) * 180.0f / M_PI;
                projectileManager.addProjectile(Projectile(position - pl::Vector2f(0, 50), angle,
                    ToolDataLoader::getProjectileTypeFromName("Serpent Venom BOSS"), 1.0f, 1.0f, HitLayer::Player));
            }
            break;
        }
        case BossSandSerpentState::MovingToPlayer:
        {
            if (!pathFollower.isActive())
            {
                behaviourState = BossSandSerpentState::IdleStage1;
            }

            // Move towards
            position = pathFollower.updateFollower(250.0f * dt);

            break;
        }
        case BossSandSerpentState::Leaving:
        {
            float playerDistance = Helper::getVectorLength(closestPlayerRelativePos - position);
            if (playerDistance < START_MOVE_PLAYER_DISTANCE)
            {
                behaviourState = BossSandSerpentState::IdleStage1;
            }

            position += Helper::normaliseVector(position - closestPlayerRelativePos) * 100.0f * dt;

            break;
        }
    }

    if (!playerAlive)
    {
        behaviourState = BossSandSerpentState::Leaving;
    }

    // Update animations
    animations[behaviourState].update(dt);

    if (behaviourState == BossSandSerpentState::IdleStage1)
    {
        headAnimation.setFrame(animations[behaviourState].getFrame());
    }

    headFlashTime -= dt;
    bodyFlashTime -= dt;

    // Update head direction
    pl::Vector2f playerPosDiff = closestPlayerRelativePos - position;
    int xWidth = animations[behaviourState].getTextureRect().width;
    
    if (std::abs(playerPosDiff.x) > std::abs(playerPosDiff.y) && std::abs(playerPosDiff.x) > xWidth / 2)
    {
        if (playerPosDiff.x > 0) headDirection = 1;
        else headDirection = -1;
    }
    else
    {
        headDirection = 0;
    }

    updateCollision();
}

void BossSandSerpent::updateCollision()
{
    headCollision = CollisionCircle(position.x, position.y - 50, HEAD_HITBOX_RADIUS);
    bodyCollision = CollisionRect(position.x - BODY_HITBOX_WIDTH / 2, position.y - BODY_HITBOX_HEIGHT, BODY_HITBOX_WIDTH, BODY_HITBOX_HEIGHT);
}

bool BossSandSerpent::takeHeadDamage(int damage, pl::Vector2f damagePosition)
{
    if (headHealth <= 0)
    {
        return false;
    }

    headFlashTime = MAX_FLASH_TIME;

    headHealth -= damage;
    HitMarkers::addHitMarker(damagePosition, damage);

    return true;
}

void BossSandSerpent::takeBodyDamage(int damage, pl::Vector2f damagePosition)
{
    if (headHealth > 0)
    {
        damage = 0;    
    }

    bodyFlashTime = MAX_FLASH_TIME;

    bodyHealth -= damage;
    HitMarkers::addHitMarker(damagePosition, damage);
}

void BossSandSerpent::applyKnockback(Projectile& projectile)
{
}

bool BossSandSerpent::isAlive()
{
    if (bodyHealth <= 0)
    {
        // Unlock achievement
        Achievements::attemptAchievementUnlock("KILLED_SANDSERPENT");
    }

    return (bodyHealth > 0);
}

// void BossSandSerpent::handleWorldWrap(pl::Vector2f positionDelta)
// {
//     position += positionDelta;

//     pathFollower.handleWorldWrap(positionDelta);
// }

void BossSandSerpent::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const pl::Color& color) const
{
    float scale = ResolutionHandler::getScale();

    switch (behaviourState)
    {
        case BossSandSerpentState::IdleStage1: // fallthrough
        case BossSandSerpentState::ShootingStage1:
        {
            pl::DrawData drawData;
            drawData.texture = TextureManager::getTexture(TextureType::Entities);
            drawData.shader = Shaders::getShader(ShaderType::Default);
            drawData.position = camera.worldToScreenTransform(position, worldSize);
            drawData.centerRatio = pl::Vector2f(0.5f, 1.0f);
            drawData.scale = pl::Vector2f(scale, scale);
            drawData.textureRect = animations.at(behaviourState).getTextureRect();

            if (bodyFlashTime > 0)
            {
                drawData.shader = Shaders::getShader(ShaderType::Flash);
                drawData.shader->setUniform1f("flash_amount", bodyFlashTime / MAX_FLASH_TIME);
            }

            // Draw body
            spriteBatch.draw(window, drawData);

            // Draw head
            if (headHealth <= 0)
            {
                break;
            }
            
            static const int HEAD_Y_OFFSET = -51;
            static const int HEAD_TEXTURE_Y_OFFSET = 32;

            drawData.position = camera.worldToScreenTransform(position + pl::Vector2f(0, HEAD_Y_OFFSET), worldSize);
            drawData.centerRatio = pl::Vector2f(0.5f, 0.5f); 

            pl::Rect<int> headTextureRect = HEAD_FRAMES[headAnimation.getFrame()];

            if (behaviourState == BossSandSerpentState::ShootingStage1)
            {
                headTextureRect = SHOOTING_HEAD_FRAME;
            }
            else
            {
                if (headDirection != 0)
                {
                    drawData.scale.x *= headDirection;
                    headTextureRect.x += HEAD_TEXTURE_Y_OFFSET;
                }
            }

            drawData.shader = Shaders::getShader(ShaderType::Default);
            drawData.textureRect = headTextureRect;

            if (headFlashTime > 0)
            {
                drawData.shader = Shaders::getShader(ShaderType::Flash);
                drawData.shader->setUniform1f("flash_amount", headFlashTime / MAX_FLASH_TIME);
            }

            spriteBatch.draw(window, drawData);
            break;
        }
        case BossSandSerpentState::MovingToPlayer: // fallthrough
        case BossSandSerpentState::Leaving:
        {
            pl::DrawData drawData;
            drawData.texture = TextureManager::getTexture(TextureType::Entities);
            drawData.shader = Shaders::getShader(ShaderType::Default);
            drawData.position = camera.worldToScreenTransform(position, worldSize);
            drawData.centerRatio = pl::Vector2f(0.5f, 1.0f);
            drawData.scale = pl::Vector2f(scale, scale);
            drawData.textureRect = animations.at(behaviourState).getTextureRect();

            // Draw tail
            spriteBatch.draw(window, drawData);
            break;
        }
    }
}

void BossSandSerpent::getHoverStats(pl::Vector2f mouseWorldPos, std::vector<std::string>& hoverStats)
{
    if (behaviourState == BossSandSerpentState::Leaving || behaviourState == BossSandSerpentState::MovingToPlayer)
    {
        return;
    }

    if (headCollision.isPointColliding(mouseWorldPos.x, mouseWorldPos.y) && headHealth > 0)
    {
        hoverStats.push_back("Sand Serpent Head (" + std::to_string(headHealth) + " / " + std::to_string(MAX_HEAD_HEALTH) + ")");
    }
    if (bodyCollision.isPointInRect(mouseWorldPos.x, mouseWorldPos.y))
    {
        hoverStats.push_back("Sand Serpent Body (" + std::to_string(bodyHealth) + " / " + std::to_string(MAX_BODY_HEALTH) + ")");
    }
}

void BossSandSerpent::testCollisionWithPlayer(Player& player, int worldSize)
{
}

void BossSandSerpent::testProjectileCollision(Projectile& projectile, int worldSize)
{
    if (behaviourState != BossSandSerpentState::IdleStage1 && behaviourState != BossSandSerpentState::ShootingStage1)
    {
        return;
    }

    // Test Collision
    if (headCollision.isColliding(projectile.getCollisionCircle(), worldSize))
    {
        if (takeHeadDamage(projectile.getDamage(), projectile.getPosition()))
        {
            projectile.onCollision();
            return;
        }
    }

    if (bodyCollision.isColliding(projectile.getCollisionCircle(), worldSize))
    {
        takeBodyDamage(projectile.getDamage(), projectile.getPosition());
        projectile.onCollision();
    }
}

void BossSandSerpent::testHitRectCollision(const std::vector<HitRect>& hitRects, int worldSize)
{
    if (headHealth > 0)
    {
        for (const HitRect& hitRect : hitRects)
        {
            if (headCollision.isColliding(hitRect, worldSize))
            {
                if (takeHeadDamage(hitRect.damage, hitRect.getCentre()))
                {
                    return;
                }
            }
        }
    }

    for (const HitRect& hitRect : hitRects)
    {
        if (bodyCollision.isColliding(hitRect, worldSize))
        {
            takeBodyDamage(hitRect.damage, hitRect.getCentre());
            break;
        }
    }
}

void BossSandSerpent::getWorldObjects(std::vector<WorldObject*>& worldObjects)
{
    worldObjects.push_back(this);
}