#include "Entity/Boss/BossEntity.hpp"
#include "Player/Player.hpp"

bool BossEntity::inPlayerRange(Player& player)
{
    return (playerMaxRange >= (player.getPosition() - position).getLength());
}

void BossEntity::createItemPickups(NetworkHandler& networkHandler, ChunkManager& chunkManager, float gameTime)
{
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

void BossEntity::setName(const std::string& name)
{
    this->name = name;
}

const std::string& BossEntity::getName()
{
    return name;
}