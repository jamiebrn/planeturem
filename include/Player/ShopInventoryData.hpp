#pragma once

#include <unordered_map>

#include "Player/InventoryData.hpp"

#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"

class ShopInventoryData : public InventoryData
{
public:
    ShopInventoryData() = default;
    ShopInventoryData(const std::vector<ItemCount>& items, const std::unordered_map<ItemType, float>& buyItemPriceMult,
        const std::unordered_map<ItemType, float>& sellItemPriceMult);

    // Get price of item to buy from inventory
    float getItemBuyPrice(ItemType itemType);

    // Get currency given if an item is sold to inventory
    float getItemSellPrice(ItemType itemType);

private:
    std::unordered_map<ItemType, float> buyItemPriceMult;
    std::unordered_map<ItemType, float> sellItemPriceMult;
};