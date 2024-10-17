#include "Player/PlayerStats.hpp"

int PlayerStats::calculateDefence(InventoryData& armourInventory)
{
    int defence = 0;

    // Iterate over armour pieces and calculate defence
    for (int i = 0; i < 3; i++)
    {
        const std::optional<ItemCount>& itemSlot = armourInventory.getItemSlotData(i);

        if (!itemSlot.has_value())
        {
            continue;
        }

        const ItemData& itemData = ItemDataLoader::getItemData(itemSlot->first);
        if (itemData.armourType < 0)
        {
            continue;
        }

        const ArmourData& armourData = ArmourDataLoader::getArmourData(itemData.armourType);

        defence += armourData.defence;
    }

    return defence;
}