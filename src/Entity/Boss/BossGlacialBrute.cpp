#include "Entity/Boss/BossGlacialBrute.hpp"
#include "Game.hpp"

const sf::IntRect BossGlacialBrute::shadowTextureRect = sf::IntRect(64, 208, 48, 16);

BossGlacialBrute::BossGlacialBrute(sf::Vector2f playerPosition, Game& game)
{
    itemDrops = {
        {{ItemDataLoader::getItemTypeFromName("Snowball Slingshot"), 1, 1}, 1.0},
        {{ItemDataLoader::getItemTypeFromName("Large Glacial Head"), 1, 1}, 0.4}
    };

    Sounds::playMusic(MusicType::BossTheme1);

    sf::Vector2i playerTile = getWorldTileInside(playerPosition, game.getChunkManager().getWorldSize());

    PathfindGridCoordinate spawnTileRelative = game.getChunkManager().getPathfindingEngine().findFurthestOpenTile(playerTile.x, playerTile.y, 40, true);

    position = sf::Vector2f(playerTile.x + spawnTileRelative.x + 0.5f, playerTile.y + spawnTileRelative.y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;

    behaviourState = BossGlacialBruteState::WalkingToPlayer;

    health = MAX_HEALTH;

    walkAnimation.create(8, 48, 64, 496, 448, 0.1);

    updateCollision();
}

void BossGlacialBrute::update(Game& game, ProjectileManager& enemyProjectileManager, Player& player, float dt)
{
    switch (behaviourState)
    {
        case BossGlacialBruteState::WalkingToPlayer:
        {
            if (!pathFollower.isActive())
            {
                const PathfindingEngine& pathfindingEngine = game.getChunkManager().getPathfindingEngine();

                int worldSize = game.getChunkManager().getWorldSize();

                sf::Vector2i tile = getWorldTileInside(worldSize);
                sf::Vector2i playerTile = player.getWorldTileInside(worldSize);

                std::vector<PathfindGridCoordinate> pathfindResult;
                if (pathfindingEngine.findPath(tile.x, tile.y, playerTile.x, playerTile.y, pathfindResult, false, 50))
                {
                    pathFollower.beginPath(position, pathfindingEngine.createStepSequenceFromPath(pathfindResult));
                }
            }
            else
            {
                sf::Vector2f beforePos = position;
                position = pathFollower.updateFollower(75.0f * dt);
                velocity = (position - beforePos) / dt;
            }

            walkAnimation.update(dt);
            break;
        }
    }

    flashTime = std::max(flashTime - dt, 0.0f);

    updateCollision();
}

bool BossGlacialBrute::isAlive()
{
    return (health > 0);
}

void BossGlacialBrute::handleWorldWrap(sf::Vector2f positionDelta)
{
    position += positionDelta;
    pathFollower.handleWorldWrap(positionDelta);
}

void BossGlacialBrute::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const sf::Color& color) const
{
    float scale = ResolutionHandler::getScale();

    // Draw shadow
    TextureDrawData drawData;
    drawData.type = TextureType::Entities;
    drawData.position = camera.worldToScreenTransform(position);
    drawData.scale = sf::Vector2f(scale, scale);
    drawData.centerRatio = sf::Vector2f(0.5f, 0.5f);

    spriteBatch.draw(window, drawData, shadowTextureRect);

    // Draw brute
    drawData.centerRatio = sf::Vector2f(0.5f, 1.0f);

    std::optional<ShaderType> shaderType = std::nullopt;

    if (flashTime > 0)
    {
        shaderType = ShaderType::Flash;
        sf::Shader* flashShader = Shaders::getShader(shaderType.value());
        flashShader->setUniform("flash_amount", flashTime / MAX_FLASH_TIME);
    }

    // Flip if required
    if (velocity.x < 0)
    {
        drawData.scale.x *= -1;
    }

    switch (behaviourState)
    {
        case BossGlacialBruteState::WalkingToPlayer:
        {
            drawData.centerRatio = sf::Vector2f(25 / 48.0f, 62 / 67.0f);
            spriteBatch.draw(window, drawData, walkAnimation.getTextureRect(), shaderType);
            break;
        }
    }
}

void BossGlacialBrute::getHoverStats(sf::Vector2f mouseWorldPos, std::vector<std::string>& hoverStats)
{
    if (hitCollision.isPointInRect(mouseWorldPos.x, mouseWorldPos.y))
    {
        hoverStats.push_back("The Glacial Brute (" + std::to_string(health) + " / " + std::to_string(MAX_HEALTH) + ")");
    }
}

void BossGlacialBrute::testCollisionWithPlayer(Player& player)
{

}

void BossGlacialBrute::testProjectileCollision(Projectile& projectile, InventoryData& inventory)
{
    if (hitCollision.isPointInRect(projectile.getPosition().x, projectile.getPosition().y))
    {
        health -= projectile.getDamage();
        flashTime = MAX_FLASH_TIME;
        HitMarkers::addHitMarker(projectile.getPosition(), projectile.getDamage());
        projectile.onCollision();
    }
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