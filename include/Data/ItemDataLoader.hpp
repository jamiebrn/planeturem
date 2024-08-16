#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <string>

#include "Core/json.hpp"
#include "Data/ItemData.hpp"

typedef unsigned int ItemType;

class ItemDataLoader
{
    ItemDataLoader() = delete;

public:
    static bool loadData(std::string itemDataPath);

    static const ItemData& getItemData(ItemType item);

private:
    static std::vector<ItemData> loaded_itemData;

};