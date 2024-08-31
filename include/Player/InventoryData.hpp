#pragma once

#include <vector>
#include <array>
#include <optional>
#include <unordered_map>

#include "Data/typedefs.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectDataLoader.hpp"

static constexpr unsigned int INVENTORY_STACK_SIZE = 20;
static constexpr unsigned int MAX_INVENTORY_SIZE = 32;

class InventoryData
{
public:
    InventoryData() = default;
    InventoryData(int size);

    void addItem(ItemType item, int amount);

    void takeItem(ItemType item, int amount);

    void addItemAtIndex(int index, ItemType item, int amount);

    void takeItemAtIndex(int index, int amount);

    // Deprecated
    // static bool canBuildObject(ObjectType object);

    std::unordered_map<ItemType, unsigned int> getTotalItemCount();

    std::optional<ItemCount>& getItemSlotData(int index);

    // inline static const std::vector<std::pair<unsigned int, int>>& getData() {return inventoryData;}

    inline int getSize() {return inventoryData.size();}

    inline const std::vector<std::optional<ItemCount>>& getData() {return inventoryData;}

private:
    std::vector<std::optional<ItemCount>> inventoryData;

    // static std::vector<std::pair<unsigned int, int>> inventoryData;

    int inventorySize;

};