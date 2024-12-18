#include "Entity/Boss/BossEntity.hpp"
#include "Player/Player.hpp"

bool BossEntity::inPlayerRange(Player& player)
{
    return (playerMaxRange >= Helper::getVectorLength(player.getPosition() - position));
}

void BossEntity::giveItemDrops(InventoryData& inventory)
{
    for (const auto& itemDropChance : itemDrops)
    {
        float randChance = (rand() % 10000) / 10000.0f;
        if (randChance > itemDropChance.second)
        {
            continue;
        }

        int amount = Helper::randInt(itemDropChance.first.minAmount, itemDropChance.first.maxAmount);

        inventory.addItem(itemDropChance.first.itemType, amount, true);
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