#include "Entity/Boss/BossEntity.hpp"
#include "Player/Player.hpp"
#include "Network/NetworkHandler.hpp"

bool BossEntity::inPlayerRange(std::vector<Player*>& players, int worldSize)
{
    for (const Player* player : players)
    {
        pl::Vector2f relativePos = Camera::translateWorldPos(player->getPosition(), position, worldSize);

        if ((relativePos - position).getLength() <= playerMaxRange)
        {
            return true;
        }
    }

    return false;
}

void BossEntity::createItemPickups(NetworkHandler& networkHandler, ChunkManager& chunkManager, float gameTime)
{
    if (networkHandler.isClient())
    {
        return;
    }

    for (const auto& itemDropChance : itemDrops)
    {
        float randChance = (rand() % 10000) / 10000.0f;
        if (randChance > itemDropChance.second)
        {
            continue;
        }

        int itemAmount = Helper::randInt(itemDropChance.first.minAmount, itemDropChance.first.maxAmount);

        for (int i = 0; i < itemAmount; i++)
        {
            pl::Vector2f spawnPos = position - pl::Vector2f(0.5f, 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
            spawnPos.x += Helper::randFloat(-itemPickupDropRadius, itemPickupDropRadius);
            spawnPos.y += Helper::randFloat(-itemPickupDropRadius, itemPickupDropRadius);

            chunkManager.addItemPickup(ItemPickup(spawnPos, itemDropChance.first.itemType, gameTime, 1), &networkHandler);
        }
    }
}

bool BossEntity::isPlayerAlive(std::vector<Player*>& players) const
{
    for (const Player* player : players)
    {
        if (player->isAlive())
        {
            return true;
        }
    }

    return false;
}

Player* BossEntity::findClosestPlayer(std::vector<Player*>& players, int worldSize, bool includeDeadPlayers) const
{
    float closestDistanceSq = std::pow(worldSize * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED, 2);
    Player* closestPlayerPtr = nullptr;

    for (Player* player : players)
    {
        if (!includeDeadPlayers && !player->isAlive())
        {
            continue;
        }
        
        pl::Vector2f relativePos = Camera::translateWorldPos(player->getPosition(), position, worldSize);
        
        float distanceSq = (relativePos - position).getLengthSq();
        
        if (distanceSq < closestDistanceSq)
        {
            closestDistanceSq = distanceSq;
            closestPlayerPtr = player;
        }
    }

    return closestPlayerPtr;
}

void BossEntity::setName(const std::string& name)
{
    this->name = name;
}

const std::string& BossEntity::getName()
{
    return name;
}