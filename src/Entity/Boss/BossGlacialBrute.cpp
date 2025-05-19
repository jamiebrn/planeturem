#include "Entity/Boss/BossGlacialBrute.hpp"
#include "Game.hpp"

const pl::Rect<int> BossGlacialBrute::shadowTextureRect = pl::Rect<int>(64, 208, 48, 16);

BossGlacialBrute::BossGlacialBrute(pl::Vector2f playerPosition, Game& game)
{
    itemDrops = {
        {{ItemDataLoader::getItemTypeFromName("Snowball Slingshot"), 1, 1}, 1.0},
        {{ItemDataLoader::getItemTypeFromName("Large Glacial Head"), 1, 1}, 0.4}
    };

    Sounds::playMusic(MusicType::BossTheme1);

    pl::Vector2<int> playerTile = getWorldTileInside(playerPosition, game.getChunkManager().getWorldSize());

    PathfindGridCoordinate spawnTileRelative = game.getChunkManager().getPathfindingEngine().findFurthestOpenTile(playerTile.x, playerTile.y, 40, true);

    position = pl::Vector2f(playerTile.x + spawnTileRelative.x + 0.5f, playerTile.y + spawnTileRelative.y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;

    behaviourState = BossGlacialBruteState::WalkingToPlayer;

    health = MAX_HEALTH;

    walkAnimation.create(8, 48, 64, 496, 448, 0.1);

    updateCollision();
}

BossEntity* BossGlacialBrute::clone() const
{
    return new BossGlacialBrute(*this);
}

void BossGlacialBrute::update(Game& game, ProjectileManager& projectileManager, std::vector<Player*>& players, float dt, int worldSize)
{
    if (game.getNetworkHandler().isLobbyHostOrSolo())
    {
        bool playerAlive = isPlayerAlive(players);
        Player* closestPlayer = findClosestPlayer(players, worldSize);
    
        switch (behaviourState)
        {
            case BossGlacialBruteState::WalkingToPlayer:
            {
                if (!pathFollower.isActive())
                {
                    const PathfindingEngine& pathfindingEngine = game.getChunkManager().getPathfindingEngine();
    
                    pl::Vector2<int> tile = getWorldTileInside(worldSize);
                    pl::Vector2<int> playerTile = closestPlayer->getWorldTileInside(worldSize);
    
                    std::vector<PathfindGridCoordinate> pathfindResult;
                    if (pathfindingEngine.findPath(tile.x, tile.y, playerTile.x, playerTile.y, pathfindResult, true, 200))
                    {
                        pathFollower.beginPath(position, pathfindingEngine.createStepSequenceFromPath(pathfindResult));
                    }
                }
                else
                {
                    pl::Vector2f beforePos = position;
                    position = pathFollower.updateFollower(75.0f * dt);
                    direction = (position - beforePos) / dt;
                }
    
                if (!playerAlive || !closestPlayer)
                {
                    behaviourState = BossGlacialBruteState::LeavingPlayer;
                    pathFollower = PathFollower();
                    break;
                }
    
                throwSnowballCooldown -= dt;
    
                if (Helper::getVectorLength(Camera::translateWorldPos(closestPlayer->getPosition(), position, worldSize) - position)
                    <= SNOWBALL_THROW_DISTANCE_THRESHOLD && throwSnowballCooldown <= 0.0f)
                {
                    throwSnowballCooldown = MAX_SNOWBALL_THROW_COOLDOWN;
                    throwSnowballTimer = Helper::randFloat(MIN_SNOWBALL_CHARGE_TIME, MAX_SNOWBALL_CHARGE_TIME);
                    behaviourState = BossGlacialBruteState::ThrowSnowball;
                    break;
                }
    
                walkAnimation.update(dt);
                break;
            }
            case BossGlacialBruteState::LeavingPlayer:
            {
                if (!pathFollower.isActive())
                {
                    pl::Vector2<int> tile = getWorldTileInside(game.getChunkManager().getWorldSize());
                    PathfindGridCoordinate furthestTile = game.getChunkManager().getPathfindingEngine().findFurthestOpenTile(tile.x, tile.y, 200);
    
                    std::vector<PathfindGridCoordinate> pathfindResult;
                    if (game.getChunkManager().getPathfindingEngine().findPath(tile.x, tile.y, furthestTile.x, furthestTile.y, pathfindResult, true, 250))
                    {
                        pathFollower.beginPath(position, game.getChunkManager().getPathfindingEngine().createStepSequenceFromPath(pathfindResult));
                    }
                }
                else
                {
                    pl::Vector2f beforePos = position;
                    position = pathFollower.updateFollower(75.0f * LEAVE_SPEED_MULT * dt);
                    direction = (position - beforePos) / dt;
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
                direction = Helper::normaliseVector(Camera::translateWorldPos(closestPlayer->getPosition(), position, worldSize) - position);
    
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

void BossGlacialBrute::throwSnowball(ProjectileManager& projectileManager, Player& player, int worldSize)
{
    pl::Vector2f playerRelativePos = Camera::translateWorldPos(player.getPosition(), position, worldSize);

    float angle = std::atan2(playerRelativePos.y - 4 - (position.y - 50), playerRelativePos.x - position.x) * 180.0f / M_PI;
    
    projectileManager.addProjectile(Projectile(position - pl::Vector2f(0, 50), angle,
        ToolDataLoader::getProjectileTypeFromName("Large Snowball"), 1.0f, 1.0f, HitLayer::Player));
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

// void BossGlacialBrute::handleWorldWrap(pl::Vector2f positionDelta)
// {
//     position += positionDelta;
//     pathFollower.handleWorldWrap(positionDelta);
// }

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