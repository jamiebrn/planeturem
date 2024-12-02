#include "Player/ShopInventoryData.hpp"

ShopInventoryData::ShopInventoryData(const std::vector<ItemCount>& items, const std::unordered_map<ItemType, float>& buyItemPriceMult,
    const std::unordered_map<ItemType, float>& sellItemPriceMult)
{
    inventoryData.clear();
    for (const ItemCount& itemCount : items)
    {
        inventoryData.push_back(itemCount);
    }

    this->buyItemPriceMult = buyItemPriceMult;
    this->sellItemPriceMult = sellItemPriceMult;
}

// Get price of item to buy from inventory
float ShopInventoryData::getItemBuyPrice(ItemType itemType)
{
    const ItemData& itemData = ItemDataLoader::getItemData(itemType);

    // Has price mult for purchasing item
    if (buyItemPriceMult.count(itemType) > 0)
    {
        return itemData.sellValue * buyItemPriceMult.at(itemType);
    }

    // Return normal price
    return itemData.sellValue;
}

// Get currency given if an item is sold to inventory
float ShopInventoryData::getItemSellPrice(ItemType itemType)
{
    const ItemData& itemData = ItemDataLoader::getItemData(itemType);

    // Has price mult for purchasing item
    if (sellItemPriceMult.count(itemType) > 0)
    {
        return itemData.sellValue * sellItemPriceMult.at(itemType);
    }

    // Return normal price
    return itemData.sellValue;
}