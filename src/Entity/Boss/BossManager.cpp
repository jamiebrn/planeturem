#include "Entity/Boss/BossManager.hpp"

void BossManager::createBoss(const std::string& name, sf::Vector2f position)
{
    // Create boss class depending on name
    if (name == "Benjamin")
    {
        bosses.push_back(std::make_unique<BossBenjaminCrow>(position));
    }
}

void BossManager::update(Game& game, ProjectileManager& projectileManager, InventoryData& inventory, sf::Vector2f playerPos, float dt)
{
    for (auto iter = bosses.begin(); iter != bosses.end();)
    {
        BossEntity* boss = iter->get();
        if (boss->isAlive())
        {
            boss->update(game, projectileManager, inventory, playerPos, dt);
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
    Sounds::stopMusic();
}

void BossManager::clearBosses()
{
    bosses.clear();
}

void BossManager::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch)
{
    for (auto& boss : bosses)
    {
        boss->draw(window, spriteBatch);
    }
}

void BossManager::drawStatsAtCursor(sf::RenderTarget& window, sf::Vector2f mouseScreenPos)
{
    for (auto& boss : bosses)
    {
        boss->drawStatsAtCursor(window, mouseScreenPos);
    }
}