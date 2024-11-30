#include "Entity/Boss/BossEntity.hpp"
#include "Player/Player.hpp"

bool BossEntity::inPlayerRange(Player& player)
{
    return (playerMaxRange >= Helper::getVectorLength(player.getPosition() - position));
}

void BossEntity::giveItemDrops(InventoryData& inventory)
{
    float randChance = (rand() % 10000) / 10000.0f;
    for (const auto& itemDropChance : itemDrops)
    {
        if (randChance > itemDropChance.second)
        {
            continue;
        }

        inventory.addItem(itemDropChance.first.first, itemDropChance.first.second);
        InventoryGUI::pushItemPopup(itemDropChance.first);
    }
}