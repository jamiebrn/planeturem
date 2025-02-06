#include "Entity/Boss/BossSandSerpent.hpp"
#include "Game.hpp"

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
        {{ItemDataLoader::getItemTypeFromName("Serpent Venom"), 55, 90}, 1.0},
        {{ItemDataLoader::getItemTypeFromName("Sandscale"), 2, 5}, 1.0},
        {{ItemDataLoader::getItemTypeFromName("Serpent Tongue"), 1, 1}, 0.4},
        {{ItemDataLoader::getItemTypeFromName("Serpent Mask"), 1, 1}, 0.2},
        {{ItemDataLoader::getItemTypeFromName("Serpent Sceptre"), 1, 1}, 0.1}
    };

    sf::Vector2i playerTile = getWorldTileInside(playerPosition, game.getChunkManager().getWorldSize());

    PathfindGridCoordinate spawnTileRelative = game.getChunkManager().getPathfindingEngine().findFurthestOpenTile(playerTile.x, playerTile.y, 40, true);

    position = sf::Vector2f(playerTile.x + spawnTileRelative.x + 0.5f, playerTile.y + spawnTileRelative.y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;

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

void BossSandSerpent::update(Game& game, ProjectileManager& enemyProjectileManager, Player& player, float dt)
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
                float angle = std::atan2(player.getPosition().y - 4 - (position.y - 50), player.getPosition().x - position.x) * 180.0f / M_PI;
                enemyProjectileManager.addProjectile(std::make_unique<Projectile>(position - sf::Vector2f(0, 50), angle,
                    ToolDataLoader::getProjectileTypeFromName("Serpent Venom BOSS"), 1.0f, 1.0f));
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

bool BossSandSerpent::takeHeadDamage(int damage, sf::Vector2f damagePosition)
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

void BossSandSerpent::takeBodyDamage(int damage, sf::Vector2f damagePosition)
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

void BossSandSerpent::handleWorldWrap(sf::Vector2f positionDelta)
{
    position += positionDelta;

    pathFollower.handleWorldWrap(positionDelta);
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

    if (headCollision.isPointColliding(mouseWorldPos.x, mouseWorldPos.y) && headHealth > 0)
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
}

void BossSandSerpent::testProjectileCollision(Projectile& projectile)
{
    if (behaviourState != BossSandSerpentState::IdleStage1 && behaviourState != BossSandSerpentState::ShootingStage1)
    {
        return;
    }

    // Test Collision
    if (headCollision.isColliding(projectile.getCollisionCircle()))
    {
        if (takeHeadDamage(projectile.getDamage(), projectile.getPosition()))
        {
            projectile.onCollision();
            return;
        }
    }

    if (bodyCollision.isColliding(projectile.getCollisionCircle()))
    {
        takeBodyDamage(projectile.getDamage(), projectile.getPosition());
        projectile.onCollision();
    }
}

void BossSandSerpent::testHitRectCollision(const std::vector<HitRect>& hitRects)
{
    if (headHealth > 0)
    {
        for (const HitRect& hitRect : hitRects)
        {
            if (headCollision.isColliding(hitRect))
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
        if (bodyCollision.isColliding(hitRect))
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