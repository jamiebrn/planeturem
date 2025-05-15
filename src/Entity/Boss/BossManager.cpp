#include "Entity/Boss/BossManager.hpp"
#include "World/ChunkManager.hpp"
#include "Entity/Boss/BossBenjaminCrow.hpp"
#include "Entity/Boss/BossSandSerpent.hpp"
#include "Entity/Boss/BossGlacialBrute.hpp"
#include "Game.hpp"

bool BossManager::createBoss(const std::string& name, pl::Vector2f playerPosition, Game& game)
{
    if (bossAliveNames.contains(name))
    {
        return false;
    }

    std::string addedBossName;

    // Create boss class depending on name
    BOSS_SPAWN(BossBenjaminCrow, "Benjamin", playerPosition)
    else BOSS_SPAWN(BossSandSerpent, "The Sand Serpent", playerPosition, game)
    else BOSS_SPAWN(BossGlacialBrute, "The Glacial Brute", playerPosition, game)

    if (!addedBossName.empty())
    {
        bosses.back()->setName(addedBossName);
        bossAliveNames.insert(addedBossName);
        return true;
    }

    return false;
}

void BossManager::update(Game& game, ProjectileManager& projectileManager, ChunkManager& chunkManager, Player& player, float dt,
    float gameTime)
{
    for (auto iter = bosses.begin(); iter != bosses.end();)
    {
        BossEntity* boss = iter->get();
        if (boss->isAlive() && boss->inPlayerRange(player))
        {
            boss->update(game, projectileManager, player, dt, chunkManager.getWorldSize());

            for (auto& projectilePair : projectileManager.getProjectiles())
            {
                if (!projectilePair.second.isAlive())
                {
                    continue;
                }
                boss->testProjectileCollision(projectilePair.second, chunkManager.getWorldSize());
            }
            iter++;
        }
        else
        {
            if (!boss->isAlive())
            {
                boss->createItemPickups(game.getNetworkHandler(), chunkManager, gameTime);
            }

            bossAliveNames.erase(boss->getName());
            iter = bosses.erase(iter);

            // Stop boss music if required
            if (bosses.size() <= 0)
            {
                stopBossMusic();
            }
        }
    }
}

void BossManager::testHitRectCollision(const std::vector<HitRect>& hitRects, int worldSize)
{
    for (auto& boss : bosses)
    {
        boss->testHitRectCollision(hitRects, worldSize);
    }
}

// void BossManager::handleWorldWrap(pl::Vector2f positionDelta)
// {
//     for (auto& boss : bosses)
//     {
//         boss->handleWorldWrap(positionDelta);
//     }   
// }

void BossManager::stopBossMusic()
{
    if (!isPlayingMusicBossMusic())
    {
        return;
    }
    
    Sounds::stopMusic();
}

bool BossManager::isPlayingMusicBossMusic()
{
    std::optional<MusicType> playingMusicType = Sounds::getPlayingMusic();
    
    if (!playingMusicType.has_value())
    {
        return false;
    }

    static constexpr std::array<MusicType, 1> bossMusicTypes = {
        MusicType::BossTheme1
    };

    for (MusicType musicType : bossMusicTypes)
    {
        if (playingMusicType.value() == musicType)
        {
            return true;
        }
    }
    
    return false;
}

void BossManager::clearBosses()
{
    bosses.clear();
    bossAliveNames.clear();
}

// void BossManager::draw(pl::RenderTarget& window, SpriteBatch& spriteBatch)
// {
//     for (auto& boss : bosses)
//     {
//         boss->draw(window, spriteBatch);
//     }
// }

void BossManager::drawStatsAtCursor(pl::RenderTarget& window, const Camera& camera, pl::Vector2f mouseScreenPos, int worldSize)
{
    std::vector<std::string> hoverStats;

    pl::Vector2f mouseWorldPos = camera.screenToWorldTransform(mouseScreenPos, worldSize);

    for (auto& boss : bosses)
    {
        boss->getHoverStats(mouseWorldPos, hoverStats);
    }

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    pl::Vector2f statPos = mouseScreenPos + pl::Vector2f(STATS_DRAW_OFFSET_X, STATS_DRAW_OFFSET_Y) * intScale;

    for (const std::string& bossStat : hoverStats)
    {
        pl::TextDrawData textDrawData;
        textDrawData.text = bossStat;
        textDrawData.position = statPos;
        textDrawData.color = pl::Color(255, 255, 255, 255);
        textDrawData.size = STATS_DRAW_SIZE * intScale;
        textDrawData.outlineColor = pl::Color(46, 34, 47);
        textDrawData.outlineThickness = STATS_DRAW_OUTLINE_THICKNESS * intScale;
        textDrawData.containOnScreenX = true;
        textDrawData.containOnScreenY = true;

        TextDraw::drawText(window, textDrawData);

        statPos.y += (STATS_DRAW_SIZE + STATS_DRAW_OUTLINE_THICKNESS * 2 + STATS_DRAW_PADDING) * intScale;
    }
}

void BossManager::getBossWorldObjects(std::vector<WorldObject*>& worldObjects)
{
    for (auto& boss : bosses)
    {
        boss->getWorldObjects(worldObjects);
    }
}

std::vector<std::unique_ptr<BossEntity>>& BossManager::getBosses()
{
    return bosses;
}

int BossManager::getBossCount() const
{
    return bosses.size();
}