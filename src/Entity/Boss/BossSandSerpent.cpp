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

BossSandSerpent::BossSandSerpent(sf::Vector2f playerPosition, Game& game)
{
    // Sounds::playSound(SoundType::Crow);
    Sounds::playMusic(MusicType::BossTheme1);

    sf::Vector2i playerTile = getWorldTileInside(playerPosition, game.getChunkManager().getWorldSize());

    PathfindGridCoordinate spawnTileRelative = game.getChunkManager().getPathfindingEngine().findFurthestOpenTile(playerTile.x, playerTile.y, 40, true);

    position = sf::Vector2f(playerTile.x + spawnTileRelative.x + 0.5f, playerTile.y + spawnTileRelative.y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;

    pathfindLastStepPosition = position;
    pathfindStepIndex = 0;

    animations[BossSandSerpentState::IdleStage1] = AnimatedTexture(4, 80, 51, 0, 301, 0.1);
    animations[BossSandSerpentState::MovingToPlayer] = AnimatedTexture(4, 16, 32, 192, 384, 0.1);

    health = MAX_HEALTH;
    dead = false;

    behaviourState = BossSandSerpentState::IdleStage1;

    updateCollision();
}

void BossSandSerpent::update(Game& game, ProjectileManager& projectileManager, InventoryData& inventory, Player& player, float dt)
{
    switch (behaviourState)
    {
        case BossSandSerpentState::IdleStage1:
        {
            float playerDistance = Helper::getVectorLength(player.getPosition() - position);
            if (playerDistance >= START_MOVE_PLAYER_DISTANCE)
            {
                // Find path to player and change behaviour state to move towards
                const PathfindingEngine& pathfindingEngine = game.getChunkManager().getPathfindingEngine();

                int worldSize = game.getChunkManager().getWorldSize();

                sf::Vector2i tile = getWorldTileInside(worldSize);
                sf::Vector2i playerTile = player.getWorldTileInside(worldSize);

                std::vector<PathfindGridCoordinate> pathfindResult;
                if (pathfindingEngine.findPath(tile.x, tile.y, playerTile.x, playerTile.y, pathfindResult))
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

    // Update animations
    animations[behaviourState].update(dt);
    headAnimation.update(dt, 1, HEAD_FRAMES.size(), 0.1);

    int worldPixelSize = game.getChunkManager().getWorldSize() * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;

    // Update head direction
    sf::Vector2f playerPosDiff = player.getPosition() - position;
    // int xWidth = animations[behaviourState].getTextureRect().width;
    
    if (std::abs(playerPosDiff.x) > std::abs(playerPosDiff.y))
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

}

void BossSandSerpent::updateCollision()
{

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

void BossSandSerpent::takeDamage(int damage, InventoryData& inventory, sf::Vector2f damagePosition)
{
    // flashTime = MAX_FLASH_TIME;

    // health -= damage;
    // HitMarkers::addHitMarker(damagePosition, damage);

    // if (health <= HEALTH_SECOND_STAGE_THRESHOLD)
    // {
    //     stage = 1;
    // }

    // if (health <= 0)
    // {
    //     // TODO: Manage item drops in boss manager on boss removal
    //     giveItemDrops(inventory);
    //     behaviourState = BossBenjaminState::Killed;

    //     // Start falling tween
    //     fallingTweenID = floatTween.startTween(&flyingHeight, flyingHeight, 0.0f, TWEEN_DEAD_FALLING_TIME, TweenTransition::Sine, TweenEasing::EaseIn);
    // }
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

void BossSandSerpent::giveItemDrops(InventoryData& inventory)
{
    // static const std::vector<ItemCount> itemDrops = {
    //     {ItemDataLoader::getItemTypeFromName("Feather"), 10},
    //     {ItemDataLoader::getItemTypeFromName("Bone"), 5},
    //     {ItemDataLoader::getItemTypeFromName("Crow Claw"), 2},
    //     {ItemDataLoader::getItemTypeFromName("Crow Skull"), 1}
    // };

    // for (const ItemCount& itemDrop : itemDrops)
    // {
    //     inventory.addItem(itemDrop.first, itemDrop.second);
    //     InventoryGUI::pushItemPopup(itemDrop);
    // }
}

bool BossSandSerpent::isProjectileColliding(Projectile& projectile)
{
    // sf::Vector2f projectilePos = projectile.getPosition();

    // return collision.isPointColliding(projectilePos.x, projectilePos.y);
    return false;
}

bool BossSandSerpent::isAlive()
{
    return (!dead);
}

void BossSandSerpent::handleWorldWrap(sf::Vector2f positionDelta)
{
    position += positionDelta;

    pathfindLastStepPosition += positionDelta;
    pathfindStepTargetPosition += positionDelta;
}

void BossSandSerpent::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, float dt, float gameTime, int worldSize, const sf::Color& color) const
{
    float scale = ResolutionHandler::getScale();

    switch (behaviourState)
    {
        case BossSandSerpentState::IdleStage1:
        {
            TextureDrawData drawData;
            drawData.type = TextureType::Entities;
            drawData.position = Camera::worldToScreenTransform(position);
            drawData.centerRatio = sf::Vector2f(0.5f, 1.0f);
            drawData.scale = sf::Vector2f(scale, scale);

            // Draw body
            spriteBatch.draw(window, drawData, animations.at(behaviourState).getTextureRect());

            // Draw head
            static const int HEAD_Y_OFFSET = -52;
            static const int HEAD_TEXTURE_Y_OFFSET = 32;

            drawData.position = Camera::worldToScreenTransform(position + sf::Vector2f(0, HEAD_Y_OFFSET));
            drawData.centerRatio = sf::Vector2f(0.5f, 0.5f); 

            sf::IntRect headTextureRect = HEAD_FRAMES[headAnimation.getFrame()];

            if (headDirection != 0)
            {
                drawData.scale.x *= headDirection;
                headTextureRect.top += HEAD_TEXTURE_Y_OFFSET;
            }

            spriteBatch.draw(window, drawData, headTextureRect);
            break;
        }
        case BossSandSerpentState::MovingToPlayer: // fallthrough
        case BossSandSerpentState::Leaving:
        {
            TextureDrawData drawData;
            drawData.type = TextureType::Entities;
            drawData.position = Camera::worldToScreenTransform(position);
            drawData.centerRatio = sf::Vector2f(0.5f, 1.0f);
            drawData.scale = sf::Vector2f(scale, scale);

            // Draw tail
            spriteBatch.draw(window, drawData, animations.at(behaviourState).getTextureRect());
            break;
        }
    }

    // spriteBatch.endDrawing(window);

    // sf::VertexArray line(sf::Lines);
    // line.append(sf::Vertex(Camera::worldToScreenTransform(pathfindLastStepPosition)));
    // line.append(sf::Vertex(Camera::worldToScreenTransform(pathfindStepTargetPosition)));
    // window.draw(line);

    // Draw pathfinding
    // for (auto coord : pathfindResult)
    // {
    //     sf::RectangleShape rect(sf::Vector2f(scale, scale) * TILE_SIZE_PIXELS_UNSCALED);
    //     rect.setPosition(Camera::worldToScreenTransform(sf::Vector2f(coord.x, coord.y) * TILE_SIZE_PIXELS_UNSCALED));
    //     rect.setFillColor(sf::Color(0, 0, 255));
    //     window.draw(rect);
    // }
}

void BossSandSerpent::drawStatsAtCursor(sf::RenderTarget& window, sf::Vector2f mouseScreenPos)
{
    // sf::Vector2f mouseWorldPos = Camera::screenToWorldTransform(mouseScreenPos);

    // if (!collision.isPointColliding(mouseWorldPos.x, mouseWorldPos.y))
    // {
    //     return;
    // }

    // float intScale = ResolutionHandler::getResolutionIntegerScale();

    // TextDrawData textDrawData;
    // textDrawData.text = "Benjamin (" + std::to_string(health) + " / " + std::to_string(MAX_HEALTH) + ")";
    // textDrawData.position = mouseScreenPos + sf::Vector2f(STATS_DRAW_OFFSET_X * intScale, STATS_DRAW_OFFSET_Y * intScale);
    // textDrawData.colour = sf::Color(255, 255, 255, 255);
    // textDrawData.size = STATS_DRAW_SIZE;
    // textDrawData.containOnScreenX = true;
    // textDrawData.containOnScreenY = true;

    // TextDraw::drawText(window, textDrawData);
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