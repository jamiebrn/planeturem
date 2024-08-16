#pragma once

#include <vector>
#include <array>
#include <optional>
#include <unordered_map>

#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/BuildRecipeLoader.hpp"

static constexpr unsigned int INVENTORY_STACK_SIZE = 20;
static constexpr unsigned int MAX_INVENTORY_SIZE = 32;

// First is item type, second is amount of item
typedef std::pair<ItemType, unsigned int> ItemCount;

class Inventory
{
    Inventory() = delete;

public:
    static void addItem(ItemType item, int amount);

    static void takeItem(ItemType item, int amount);

    static void addItemAtIndex(int index, ItemType item, int amount);

    static void takeItemAtIndex(int index, int amount);

    static bool canBuildObject(ObjectType object);

    static std::unordered_map<ItemType, unsigned int> getTotalItemCount();

    // inline static const std::vector<std::pair<unsigned int, int>>& getData() {return inventoryData;}

    inline static const std::array<std::optional<ItemCount>, MAX_INVENTORY_SIZE>& getData() {return inventoryData;}

private:
    static std::array<std::optional<ItemCount>, MAX_INVENTORY_SIZE> inventoryData;

    // static std::vector<std::pair<unsigned int, int>> inventoryData;

    static int inventorySize;

};