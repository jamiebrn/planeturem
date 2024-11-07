#include "Entity/Boss/BossManager.hpp"

void BossManager::createBoss(const std::string& name, sf::Vector2f playerPosition, Game& game)
{
    // Create boss class depending on name
    if (name == "Benjamin")
    {
        bosses.push_back(std::make_unique<BossBenjaminCrow>(playerPosition));
    }
    else if (name == "The Sand Serpent")
    {
        bosses.push_back(std::make_unique<BossSandSerpent>(playerPosition, game));
    }
}

void BossManager::update(Game& game, ProjectileManager& projectileManager, InventoryData& inventory, Player& player, float dt)
{
    for (auto iter = bosses.begin(); iter != bosses.end();)
    {
        BossEntity* boss = iter->get();
        if (boss->isAlive() && boss->inPlayerRange(player))
        {
            boss->update(game, projectileManager, inventory, player, dt);
            iter++;
        }
        else
        {
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
}

// void BossManager::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch)
// {
//     for (auto& boss : bosses)
//     {
//         boss->draw(window, spriteBatch);
//     }
// }

void BossManager::drawStatsAtCursor(sf::RenderTarget& window, sf::Vector2f mouseScreenPos)
{
    std::vector<std::string> hoverStats;

    for (auto& boss : bosses)
    {
        boss->getHoverStats(mouseScreenPos, hoverStats);
    }

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    sf::Vector2f statPos = mouseScreenPos + sf::Vector2f(STATS_DRAW_OFFSET_X * intScale, STATS_DRAW_OFFSET_Y * intScale);

    for (const std::string& bossStat : hoverStats)
    {
        TextDrawData textDrawData;
        textDrawData.text = bossStat;
        textDrawData.position = statPos;
        textDrawData.colour = sf::Color(255, 255, 255, 255);
        textDrawData.size = STATS_DRAW_SIZE;
        textDrawData.containOnScreenX = true;
        textDrawData.containOnScreenY = true;

        TextDraw::drawText(window, textDrawData);

        statPos.y += (STATS_DRAW_SIZE + STATS_DRAW_PADDING) * intScale;
    }
}

void BossManager::getBossWorldObjects(std::vector<WorldObject*>& worldObjects)
{
    for (auto& boss : bosses)
    {
        boss->getWorldObjects(worldObjects);
    }
}