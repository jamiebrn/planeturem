#pragma once

#include <vector>
#include <array>
#include <optional>
#include <unordered_map>

#include "Data/typedefs.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectDataLoader.hpp"

// static constexpr unsigned int INVENTORY_STACK_SIZE = 20;
// static constexpr unsigned int MAX_INVENTORY_SIZE = 32;

class InventoryData
{
public:
    InventoryData() = default;
    InventoryData(int size);

    int addItem(ItemType item, int amount);

    void takeItem(ItemType item, int amount);

    void addItemAtIndex(int index, ItemType item, int amount);

    void takeItemAtIndex(int index, int amount);

    std::unordered_map<ItemType, unsigned int> getTotalItemCount() const;

    std::optional<ItemCount>& getItemSlotData(int index);

    inline int getSize() {return inventoryData.size();}

    inline const std::vector<std::optional<ItemCount>>& getData() {return inventoryData;}

private:
    std::vector<std::optional<ItemCount>> inventoryData;

    int inventorySize;

};