#pragma once

#include <vector>
#include <array>
#include <optional>
#include <unordered_map>
#include <fstream>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/utility.hpp>
#include <extlib/cereal/types/optional.hpp>

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

    bool isEmpty() const;

    inline int getSize() {return inventoryData.size();}

    inline const std::vector<std::optional<ItemCount>>& getData() {return inventoryData;}

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(inventoryData);
    }

private:
    std::vector<std::optional<ItemCount>> inventoryData;

};