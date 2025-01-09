#include "Entity/Boss/BossManager.hpp"
#include "Entity/Boss/BossBenjaminCrow.hpp"
#include "Entity/Boss/BossSandSerpent.hpp"
#include "Entity/Boss/BossGlacialBrute.hpp"

bool BossManager::createBoss(const std::string& name, sf::Vector2f playerPosition, Game& game)
{
    if (bossAliveNames.contains(name))
    {
        return false;
    }

    std::string addedBossName;

    // Create boss class depending on name
    if (std::string bossName = "Benjamin"; name == bossName)
    {
        bosses.push_back(std::make_unique<BossBenjaminCrow>(playerPosition));
        addedBossName = bossName;
    }
    else if (std::string bossName = "The Sand Serpent"; name == bossName)
    {
        bosses.push_back(std::make_unique<BossSandSerpent>(playerPosition, game));
        addedBossName = bossName;
    }
    // Removed in demo
    // else if (std::string bossName = "The Glacial Brute"; name == bossName)
    // {
    //     bosses.push_back(std::make_unique<BossGlacialBrute>(playerPosition, game));
    //     addedBossName = bossName;
    // }

    if (!addedBossName.empty())
    {
        bosses.back()->setName(addedBossName);
        bossAliveNames.insert(addedBossName);
        return true;
    }

    return false;
}

void BossManager::update(Game& game, ProjectileManager& projectileManager, ProjectileManager& enemyProjectileManager, InventoryData& inventory, Player& player, float dt)
{
    for (auto iter = bosses.begin(); iter != bosses.end();)
    {
        BossEntity* boss = iter->get();
        if (boss->isAlive() && boss->inPlayerRange(player))
        {
            boss->update(game, enemyProjectileManager, player, dt);

            for (auto& projectile : projectileManager.getProjectiles())
            {
                if (!projectile->isAlive())
                {
                    continue;
                }
                boss->testProjectileCollision(*projectile, inventory);
            }
            iter++;
        }
        else
        {
            if (!boss->isAlive())
            {
                boss->giveItemDrops(inventory);
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

void BossManager::handleWorldWrap(sf::Vector2f positionDelta)
{
    for (auto& boss : bosses)
    {
        boss->handleWorldWrap(positionDelta);
    }   
}

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

// void BossManager::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch)
// {
//     for (auto& boss : bosses)
//     {
//         boss->draw(window, spriteBatch);
//     }
// }

void BossManager::drawStatsAtCursor(sf::RenderTarget& window, const Camera& camera, sf::Vector2f mouseScreenPos)
{
    std::vector<std::string> hoverStats;

    sf::Vector2f mouseWorldPos = camera.screenToWorldTransform(mouseScreenPos);

    for (auto& boss : bosses)
    {
        boss->getHoverStats(mouseWorldPos, hoverStats);
    }

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    sf::Vector2f statPos = mouseScreenPos + sf::Vector2f(STATS_DRAW_OFFSET_X, STATS_DRAW_OFFSET_Y) * intScale;

    for (const std::string& bossStat : hoverStats)
    {
        TextDrawData textDrawData;
        textDrawData.text = bossStat;
        textDrawData.position = statPos;
        textDrawData.colour = sf::Color(255, 255, 255, 255);
        textDrawData.size = STATS_DRAW_SIZE * intScale;
        textDrawData.outlineColour = sf::Color(46, 34, 47);
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