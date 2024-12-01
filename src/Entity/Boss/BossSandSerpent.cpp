#include "Entity/Boss/BossSandSerpent.hpp"
#include "Game.hpp"

// const int BossSandSerpent::DAMAGE_HITBOX_SIZE = 20;
// const std::array<int, 2> BossSandSerpent::damageValues = {35, 65};

// const std::array<sf::IntRect, 2> BossSandSerpent::dashGhostTextureRects = {
//     sf::IntRect(400, 160, 48, 64),
//     sf::IntRect(400, 224, 48, 64)
// };
// const sf::IntRect BossSandSerpent::deadTextureRect = sf::IntRect(448, 160, 48, 64);
// const sf::IntRect BossSandSerpent::shadowTextureRect = sf::IntRect(64, 208, 48, 16);

const std::array<sf::IntRect, 4> BossSandSerpent::HEAD_FRAMES = {
    sf::IntRect(0, 384, 48, 32),
    sf::IntRect(48, 384, 48, 32),
    sf::IntRect(96, 384, 48, 32),
    sf::IntRect(144, 384, 48, 32),
};

const sf::IntRect BossSandSerpent::SHOOTING_HEAD_FRAME = sf::IntRect(192, 416, 48, 32);

BossSandSerpent::BossSandSerpent(sf::Vector2f playerPosition, Game& game)
{
    // Sounds::playSound(SoundType::Crow);
    Sounds::playMusic(MusicType::BossTheme1);

    itemDrops = {
        {{ItemDataLoader::getItemTypeFromName("Serpent Venom"), 75}, 1.0},
        {{ItemDataLoader::getItemTypeFromName("Sandscale"), 3}, 1.0},
        {{ItemDataLoader::getItemTypeFromName("Serpent Tongue"), 1}, 0.4},
        {{ItemDataLoader::getItemTypeFromName("Serpent Mask"), 1}, 0.2},
        {{ItemDataLoader::getItemTypeFromName("Serpent Sceptre"), 1}, 0.1}
    };

    sf::Vector2i playerTile = getWorldTileInside(playerPosition, game.getChunkManager().getWorldSize());

    PathfindGridCoordinate spawnTileRelative = game.getChunkManager().getPathfindingEngine().findFurthestOpenTile(playerTile.x, playerTile.y, 40, true);

    position = sf::Vector2f(playerTile.x + spawnTileRelative.x + 0.5f, playerTile.y + spawnTileRelative.y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;

    pathfindLastStepPosition = position;
    pathfindStepIndex = 0;

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

void BossSandSerpent::update(Game& game, ProjectileManager& projectileManager, ProjectileManager& enemyProjectileManager, InventoryData& inventory, Player& player, float dt)
{
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

            float playerDistance = Helper::getVectorLength(player.getPosition() - position);
            if (playerDistance >= START_MOVE_PLAYER_DISTANCE)
            {
                // Find path to player and change behaviour state to move towards
                const PathfindingEngine& pathfindingEngine = game.getChunkManager().getPathfindingEngine();

                int worldSize = game.getChunkManager().getWorldSize();

                sf::Vector2i tile = getWorldTileInside(worldSize);
                sf::Vector2i playerTile = player.getWorldTileInside(worldSize);

                std::vector<PathfindGridCoordinate> pathfindResult;
                if (pathfindingEngine.findPath(tile.x, tile.y, playerTile.x, playerTile.y, pathfindResult, false, 50))
                {
                    pathfindStepSequence = pathfindingEngine.createStepSequenceFromPath(pathfindResult);

                    pathfindLastStepPosition = position;
                    setPathfindStepIndex(0);  

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
                float angle = std::atan2(player.getPosition().y - 4 - (position.y - 50), player.getPosition().x - position.x) * 180.0f / M_PI;
                enemyProjectileManager.addProjectile(std::make_unique<Projectile>(position - sf::Vector2f(0, 50), angle,
                    ToolDataLoader::getProjectileTypeFromName("Serpent Venom"), 1.0f, 1.0f));
            }
            break;
        }
        case BossSandSerpentState::MovingToPlayer:
        {
            if (pathfindStepIndex < pathfindStepSequence.size())
            {
                if (Helper::getVectorLength(pathfindStepTargetPosition - position) <= 2.0f)
                {
                    // Move to next pathfinding step
                    pathfindLastStepPosition = pathfindStepTargetPosition;
                    setPathfindStepIndex(pathfindStepIndex + 1);
                }
            }
            else
            {
                behaviourState = BossSandSerpentState::IdleStage1;
            }

            // Move towards
            position += Helper::normaliseVector(pathfindStepTargetPosition - position) * 250.0f * dt;

            break;
        }
        case BossSandSerpentState::Leaving:
        {
            float playerDistance = Helper::getVectorLength(player.getPosition() - position);
            if (playerDistance < START_MOVE_PLAYER_DISTANCE)
            {
                behaviourState = BossSandSerpentState::IdleStage1;
            }

            position += Helper::normaliseVector(position - player.getPosition()) * 100.0f * dt;

            break;
        }
    }

    if (!player.isAlive())
    {
        behaviourState = BossSandSerpentState::Leaving;
    }

    if (behaviourState == BossSandSerpentState::IdleStage1 || behaviourState == BossSandSerpentState::ShootingStage1)
    {
        // Test Collision
        for (auto& projectile : projectileManager.getProjectiles())
        {
            sf::Vector2f projectilePos = projectile->getPosition();
            if (headCollision.isPointColliding(projectilePos.x, projectilePos.y))
            {
                if (takeHeadDamage(projectile->getDamage(), inventory, projectile->getPosition()))
                {
                    projectile->onCollision();
                    continue;
                }
            }
            if (bodyCollision.isPointInRect(projectilePos.x, projectilePos.y))
            {
                takeBodyDamage(projectile->getDamage(), inventory, projectile->getPosition());
                projectile->onCollision();
            }
        }
    }

    // Update animations
    animations[behaviourState].update(dt);

    if (behaviourState == BossSandSerpentState::IdleStage1)
    {
        headAnimation.setFrame(animations[behaviourState].getFrame());
    }

    headFlashTime -= dt;
    bodyFlashTime -= dt;

    int worldPixelSize = game.getChunkManager().getWorldSize() * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;

    // Update head direction
    sf::Vector2f playerPosDiff = player.getPosition() - position;
    int xWidth = animations[behaviourState].getTextureRect().width;
    
    if (std::abs(playerPosDiff.x) > std::abs(playerPosDiff.y) && std::abs(playerPosDiff.x) > xWidth / 2)
    {
        if (playerPosDiff.x > 0) headDirection = 1;
        else headDirection = -1;

        if (std::abs(playerPosDiff.x) >= worldPixelSize / 2)
        {
            // Handle world repeating case
            headDirection *= -1;
        }
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

void BossSandSerpent::setPathfindStepIndex(int index)
{
    pathfindStepIndex = index;

    if (pathfindStepIndex >= pathfindStepSequence.size())
    {
        return;
    }

    PathfindGridCoordinate target = pathfindStepSequence[pathfindStepIndex];
    sf::Vector2i tilePosition = getTileInside(pathfindLastStepPosition);
    pathfindStepTargetPosition = sf::Vector2f(target.x + tilePosition.x + 0.5f, target.y + tilePosition.y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
}

bool BossSandSerpent::takeHeadDamage(int damage, InventoryData& inventory, sf::Vector2f damagePosition)
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

void BossSandSerpent::takeBodyDamage(int damage, InventoryData& inventory, sf::Vector2f damagePosition)
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
    // if (behaviourState == BossBenjaminState::Dash)
    // {
    //     return;
    // }

    // sf::Vector2f relativePos = sf::Vector2f(position.x, position.y - flyingHeight) - projectile.getPosition();

    // static constexpr float KNOCKBACK_STRENGTH = 7.0f;

    // velocity -= Helper::normaliseVector(-relativePos) * KNOCKBACK_STRENGTH;
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

void BossSandSerpent::handleWorldWrap(sf::Vector2f positionDelta)
{
    position += positionDelta;

    pathfindLastStepPosition += positionDelta;
    pathfindStepTargetPosition += positionDelta;
}

void BossSandSerpent::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const sf::Color& color) const
{
    float scale = ResolutionHandler::getScale();

    switch (behaviourState)
    {
        case BossSandSerpentState::IdleStage1: // fallthrough
        case BossSandSerpentState::ShootingStage1:
        {
            TextureDrawData drawData;
            drawData.type = TextureType::Entities;
            drawData.position = camera.worldToScreenTransform(position);
            drawData.centerRatio = sf::Vector2f(0.5f, 1.0f);
            drawData.scale = sf::Vector2f(scale, scale);

            std::optional<ShaderType> shader;
            if (bodyFlashTime > 0)
            {
                shader = ShaderType::Flash;
                Shaders::getShader(shader.value())->setUniform("flash_amount", bodyFlashTime / MAX_FLASH_TIME);
            }

            // Draw body
            spriteBatch.draw(window, drawData, animations.at(behaviourState).getTextureRect(), shader);

            // Draw head
            if (headHealth <= 0)
            {
                break;
            }
            
            static const int HEAD_Y_OFFSET = -51;
            static const int HEAD_TEXTURE_Y_OFFSET = 32;

            drawData.position = camera.worldToScreenTransform(position + sf::Vector2f(0, HEAD_Y_OFFSET));
            drawData.centerRatio = sf::Vector2f(0.5f, 0.5f); 

            sf::IntRect headTextureRect = HEAD_FRAMES[headAnimation.getFrame()];

            if (behaviourState == BossSandSerpentState::ShootingStage1)
            {
                headTextureRect = SHOOTING_HEAD_FRAME;
            }
            else
            {
                if (headDirection != 0)
                {
                    drawData.scale.x *= headDirection;
                    headTextureRect.top += HEAD_TEXTURE_Y_OFFSET;
                }
            }

            shader = std::nullopt;
            if (headFlashTime > 0)
            {
                shader = ShaderType::Flash;
                Shaders::getShader(shader.value())->setUniform("flash_amount", headFlashTime / MAX_FLASH_TIME);
            }

            spriteBatch.draw(window, drawData, headTextureRect, shader);
            break;
        }
        case BossSandSerpentState::MovingToPlayer: // fallthrough
        case BossSandSerpentState::Leaving:
        {
            TextureDrawData drawData;
            drawData.type = TextureType::Entities;
            drawData.position = camera.worldToScreenTransform(position);
            drawData.centerRatio = sf::Vector2f(0.5f, 1.0f);
            drawData.scale = sf::Vector2f(scale, scale);

            // Draw tail
            spriteBatch.draw(window, drawData, animations.at(behaviourState).getTextureRect());
            break;
        }
    }
}

void BossSandSerpent::getHoverStats(sf::Vector2f mouseWorldPos, std::vector<std::string>& hoverStats)
{
    if (behaviourState == BossSandSerpentState::Leaving || behaviourState == BossSandSerpentState::MovingToPlayer)
    {
        return;
    }

    if (headCollision.isPointColliding(mouseWorldPos.x, mouseWorldPos.y))
    {
        hoverStats.push_back("Sand Serpent Head (" + std::to_string(headHealth) + " / " + std::to_string(MAX_HEAD_HEALTH) + ")");
    }
    if (bodyCollision.isPointInRect(mouseWorldPos.x, mouseWorldPos.y))
    {
        hoverStats.push_back("Sand Serpent Body (" + std::to_string(bodyHealth) + " / " + std::to_string(MAX_BODY_HEALTH) + ")");
    }
}

void BossSandSerpent::testCollisionWithPlayer(Player& player)
{
    // if (behaviourState == BossBenjaminState::Killed)
    // {
    //     return;
    // }

    // HitRect hitRect;
    // hitRect.x = position.x - DAMAGE_HITBOX_SIZE;
    // hitRect.y = position.y - flyingHeight - DAMAGE_HITBOX_SIZE;
    // hitRect.height = DAMAGE_HITBOX_SIZE * 2;
    // hitRect.width = DAMAGE_HITBOX_SIZE * 2;

    // hitRect.damage = damageValues[stage];

    // player.testHitCollision(hitRect);
}

void BossSandSerpent::getWorldObjects(std::vector<WorldObject*>& worldObjects)
{
    worldObjects.push_back(this);
}