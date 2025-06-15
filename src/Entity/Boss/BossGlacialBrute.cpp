#include "Entity/Boss/BossGlacialBrute.hpp"
#include "Game.hpp"

const pl::Rect<int> BossGlacialBrute::shadowTextureRect = pl::Rect<int>(64, 208, 48, 16);

BossGlacialBrute::BossGlacialBrute()
{
    initialise();
}

BossGlacialBrute::BossGlacialBrute(pl::Vector2f playerPosition, Game& game, ChunkManager& chunkManager)
{
    itemDrops = {
        {{ItemDataLoader::getItemTypeFromName("Snowball Slingshot"), 1, 1}, 1.0},
        {{ItemDataLoader::getItemTypeFromName("Large Glacial Head"), 1, 1}, 0.4}
    };

    Sounds::playMusic(MusicType::BossTheme1);

    pl::Vector2<int> playerTile = getWorldTileInside(playerPosition, chunkManager.getWorldSize());

    PathfindGridCoordinate spawnTileRelative = chunkManager.getPathfindingEngine().findFurthestOpenTile(playerTile.x, playerTile.y, 40, true);

    position = pl::Vector2f(playerTile.x + spawnTileRelative.x + 0.5f, playerTile.y + spawnTileRelative.y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;

    Helper::wrapPosition(position, chunkManager.getWorldSize());

    behaviourState = BossGlacialBruteState::WalkingToPlayer;

    health = MAX_HEALTH;

    initialise();
}

void BossGlacialBrute::initialise()
{
    walkAnimation.create(8, 48, 80, 496, 592, 0.1f);
    cannonWalkAnimation.create(8, 80, 64, 384, 672, 0.1f);
    
    updateCollision();
}

BossEntity* BossGlacialBrute::clone() const
{
    return new BossGlacialBrute(*this);
}

void BossGlacialBrute::update(Game& game, ChunkManager& chunkManager, ProjectileManager& projectileManager, std::vector<Player*>& players, float dt, int worldSize)
{
    if (game.getNetworkHandler().isLobbyHostOrSolo())
    {
        bool playerAlive = isPlayerAlive(players);
        Player* closestPlayer = findClosestPlayer(players, worldSize);
    
        switch (behaviourState)
        {
            case BossGlacialBruteState::WalkingToPlayer: // fallthrough
            case BossGlacialBruteState::SnowballCannon:
            {
                if (!playerAlive || !closestPlayer)
                {
                    behaviourState = BossGlacialBruteState::LeavingPlayer;
                    pathFollower = PathFollower();
                    break;
                }

                if (health < HEALTH_SNOWBALL_CANNON_THRESHOLD)
                {
                    behaviourState = BossGlacialBruteState::SnowballCannon;
                }

                throwSnowballCooldown -= dt;

                if (behaviourState == BossGlacialBruteState::WalkingToPlayer)
                {
                    walkAnimation.update(dt);
    
                    if (Helper::getVectorLength(Camera::translateWorldPos(closestPlayer->getPosition(), position, worldSize) - position)
                        <= SNOWBALL_THROW_DISTANCE_THRESHOLD && throwSnowballCooldown <= 0.0f)
                    {
                        throwSnowballCooldown = MAX_SNOWBALL_THROW_COOLDOWN;
                        throwSnowballTimer = Helper::randFloat(MIN_SNOWBALL_CHARGE_TIME, MAX_SNOWBALL_CHARGE_TIME);
                        behaviourState = BossGlacialBruteState::ThrowSnowball;
                        break;
                    }
                }
                else
                {
                    cannonWalkAnimation.update(dt);

                    if (throwSnowballCooldown <= 0.0f)
                    {
                        throwSnowballCooldown = MAX_SHOOT_SNOWBALL_COOLDOWN;
                        shootSnowball(projectileManager, *closestPlayer, worldSize);
                    }
                }

                pl::Vector2<uint32_t> playerTile = closestPlayer->getWorldTileInside(worldSize);

                const PathfindingEngine& pathfindingEngine = chunkManager.getPathfindingEngine();

                if (!pathFollower.isActive() || !pathfindingEngine.isPathFollowerValid(pathFollower) ||
                    playerTile.x != targetPathfindGridCoordinate.x || playerTile.y != targetPathfindGridCoordinate.y)
                {
                    pl::Vector2<uint32_t> tile = getWorldTileInside(worldSize);
    
                    std::vector<PathfindGridCoordinate> pathfindResult;
                    if (pathfindingEngine.findPath(tile.x, tile.y, playerTile.x, playerTile.y, pathfindResult, true, 200))
                    {
                        pathFollower.beginPath(position, pathfindingEngine.createStepSequenceFromPath(pathfindResult), pathfindingEngine);
                        targetPathfindGridCoordinate = pathfindResult[0];
                    }
                }
                else
                {
                    pl::Vector2f beforePos = position;
                    position = pathFollower.updateFollower(75.0f * dt);
                    velocity = (position - beforePos) / dt;
                    direction = velocity.normalise();
                }
                break;
            }
            case BossGlacialBruteState::LeavingPlayer:
            {
                if (!pathFollower.isActive())
                {
                    pl::Vector2<int> tile = getWorldTileInside(chunkManager.getWorldSize());
                    PathfindGridCoordinate furthestTile = chunkManager.getPathfindingEngine().findFurthestOpenTile(tile.x, tile.y, 200);
    
                    std::vector<PathfindGridCoordinate> pathfindResult;
                    if (chunkManager.getPathfindingEngine().findPath(tile.x, tile.y, furthestTile.x, furthestTile.y, pathfindResult, true, 250))
                    {
                        pathFollower.beginPath(position, chunkManager.getPathfindingEngine().createStepSequenceFromPath(pathfindResult),
                            chunkManager.getPathfindingEngine());
                    }
                }
                else
                {
                    pl::Vector2f beforePos = position;
                    position = pathFollower.updateFollower(75.0f * LEAVE_SPEED_MULT * dt);
                    velocity = (position - beforePos) / dt;
                    direction = velocity.normalise();
                }
    
                if (playerAlive)
                {
                    behaviourState = BossGlacialBruteState::WalkingToPlayer;
                    pathFollower = PathFollower();
                }
    
                walkAnimation.update(dt);
                break;
            }
            case BossGlacialBruteState::ThrowSnowball:
            {
                velocity = pl::Vector2f(0, 0);
                direction = (Camera::translateWorldPos(closestPlayer->getPosition(), position, worldSize) - position).normalise();
    
                throwSnowballTimer -= dt;
    
                if (throwSnowballTimer <= 0)
                {
                    throwSnowball(projectileManager, *closestPlayer, worldSize);
                    behaviourState = BossGlacialBruteState::WalkingToPlayer;
                }
                break;
            }
        }
    }

    Helper::wrapPosition(position, worldSize);

    flashTime = std::max(flashTime - dt, 0.0f);

    updateCollision();
}

void BossGlacialBrute::updateNetwork(Player& player, float dt, int worldSize)
{
    position += velocity * dt;

    Helper::wrapPosition(position, worldSize);

    flashTime = std::max(flashTime - dt, 0.0f);

    updateCollision();
}

void BossGlacialBrute::throwSnowball(ProjectileManager& projectileManager, Player& player, int worldSize)
{
    pl::Vector2f playerRelativePos = Camera::translateWorldPos(player.getPosition(), position, worldSize);

    float angle = std::atan2(playerRelativePos.y - 4 - (position.y - 50), playerRelativePos.x - position.x) * 180.0f / M_PI;
    
    projectileManager.addProjectile(Projectile(position - pl::Vector2f(0, 50), angle,
        ToolDataLoader::getProjectileTypeFromName("Large Snowball"), 1.0f, 1.0f, HitLayer::Player));
}

void BossGlacialBrute::shootSnowball(ProjectileManager& projectileManager, Player& player, int worldSize)
{
    int xScale = (direction.x < 0) ? -1 : 1;
    pl::Vector2f cannonEndPos = position + pl::Vector2f(29 * xScale, -24);

    pl::Vector2f playerRelativePos = Camera::translateWorldPos(player.getPosition(), cannonEndPos, worldSize);

    float angle = std::atan2(playerRelativePos.y - 4 - (cannonEndPos.y - 50), playerRelativePos.x - cannonEndPos.x) * 180.0f / M_PI;

    static constexpr float SHOOT_ANGLE_VARIATION = 10.0f;
    angle += Helper::randFloat(-SHOOT_ANGLE_VARIATION, SHOOT_ANGLE_VARIATION);

    float SPREAD_ANGLE_DELTA = 20.0f;

    int projectileMax = 1;
    if (health <= HEALTH_SNOWBALL_CANNON_AGGRESSIVE_THRESHOLD)
    {
        projectileMax = 2;
        SPREAD_ANGLE_DELTA *= 0.66f;
    }

    for (int i = -projectileMax; i <= projectileMax; i++)
    {
        projectileManager.addProjectile(Projectile(cannonEndPos, angle + SPREAD_ANGLE_DELTA * i,
            ToolDataLoader::getProjectileTypeFromName("Snowball"), 1.0f, 1.0f, HitLayer::Player));
    }
}

bool BossGlacialBrute::isAlive()
{
    if (health <= 0)
    {
        // Unlock achievement
        Achievements::attemptAchievementUnlock("KILLED_GLACIAL_BRUTE");
    }

    return (health > 0);
}

void BossGlacialBrute::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const pl::Color& color) const
{
    float scale = ResolutionHandler::getScale();

    // Draw shadow
    pl::DrawData drawData;
    drawData.texture = TextureManager::getTexture(TextureType::Entities);
    drawData.shader = Shaders::getShader(ShaderType::Default);
    drawData.position = camera.worldToScreenTransform(position, worldSize);
    drawData.scale = pl::Vector2f(scale, scale);
    drawData.centerRatio = pl::Vector2f(0.5f, 0.5f);
    drawData.textureRect = shadowTextureRect;

    spriteBatch.draw(window, drawData);

    // Draw brute
    drawData.centerRatio = pl::Vector2f(0.5f, 1.0f);

    std::optional<ShaderType> shaderType = std::nullopt;

    if (flashTime > 0)
    {
        drawData.shader = Shaders::getShader(ShaderType::Flash);
        drawData.shader->setUniform1f("flash_amount", flashTime / MAX_FLASH_TIME);
    }

    // Flip if required
    if (direction.x < 0)
    {
        drawData.scale.x *= -1;
    }

    switch (behaviourState)
    {
        case BossGlacialBruteState::WalkingToPlayer: // fallthrough
        case BossGlacialBruteState::LeavingPlayer:
        {
            drawData.centerRatio = pl::Vector2f(25 / 48.0f, 62 / 67.0f);
            drawData.textureRect = walkAnimation.getTextureRect();
            break;
        }
        case BossGlacialBruteState::SnowballCannon:
        {
            drawData.centerRatio = pl::Vector2f(41 / 80.0f, 62 / 67.0f);
            drawData.textureRect = cannonWalkAnimation.getTextureRect();
            break;
        }
        case BossGlacialBruteState::ThrowSnowball:
        {
            drawData.centerRatio = pl::Vector2f(25 / 48.0f, 62 / 67.0f);
            drawData.textureRect = pl::Rect<int>(496, 512, 48, 80);
            break;
        }
    }

    spriteBatch.draw(window, drawData);
}

void BossGlacialBrute::getHoverStats(pl::Vector2f mouseWorldPos, std::vector<std::string>& hoverStats)
{
    if (hitCollision.isPointInRect(mouseWorldPos.x, mouseWorldPos.y))
    {
        hoverStats.push_back("The Glacial Brute (" + std::to_string(health) + " / " + std::to_string(MAX_HEALTH) + ")");
    }
}

void BossGlacialBrute::testCollisionWithPlayer(Player& player, int worldSize)
{
    HitRect hitRect(hitCollision);
    hitRect.damage = BODY_DAMAGE;
    player.testHitCollision(hitRect, worldSize);
}

void BossGlacialBrute::testProjectileCollision(Projectile& projectile, int worldSize)
{
    if (projectile.getHitLayer() != HitLayer::Entity)
    {
        return;
    }
    
    if (hitCollision.isColliding(projectile.getCollisionCircle(), worldSize))
    {
        damage(projectile.getDamage(), projectile.getPosition());
        projectile.onCollision();
    }
}

void BossGlacialBrute::testHitRectCollision(const std::vector<HitRect>& hitRects, int worldSize)
{
    for (const HitRect& hitRect : hitRects)
    {
        if (hitCollision.isColliding(hitRect, worldSize))
        {
            damage(hitRect.damage, hitRect.getCentre());
            return;
        }   
    }
}

void BossGlacialBrute::damage(int amount, pl::Vector2f damageSource)
{
    health -= amount;
    flashTime = MAX_FLASH_TIME;
    HitMarkers::addHitMarker(damageSource, amount);
}

void BossGlacialBrute::getWorldObjects(std::vector<WorldObject*>& worldObjects)
{
    worldObjects.push_back(this);
}

void BossGlacialBrute::updateCollision()
{
    hitCollision.x = position.x - 17;
    hitCollision.y = position.y - 61;
    hitCollision.width = 29;
    hitCollision.height = 61;
}