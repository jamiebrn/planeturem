#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <string>
#include <unordered_map>

#include "Core/json.hpp"
#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"

class ItemDataLoader
{
    ItemDataLoader() = delete;

public:
    static bool loadData(std::string itemDataPath);

    static const ItemData& getItemData(ItemType item);

    static ItemType getItemTypeFromName(const std::string& itemName);

    static void createItemFromObject(const std::string& objectName, ObjectType placesObject);

private:
    static std::vector<ItemData> loaded_itemData;

    static std::unordered_map<std::string, ItemType> itemNameToTypeMap;

};