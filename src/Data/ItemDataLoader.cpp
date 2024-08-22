#include "Data/ItemDataLoader.hpp"

std::vector<ItemData> ItemDataLoader::loaded_itemData;
std::unordered_map<std::string, ItemType> ItemDataLoader::itemNameToTypeMap;

bool ItemDataLoader::loadData(std::string itemDataPath)
{
    std::ifstream file(itemDataPath);
    nlohmann::json data = nlohmann::json::parse(file);

    int itemIndex = 0;

    // Load data
    for (nlohmann::json::iterator iter = data.begin(); iter != data.end(); ++iter)
    {
        ItemData itemData;
        auto jsonItemData = iter.value();

        itemData.name = jsonItemData.at("name");

        if (jsonItemData.contains("texture-x")) itemData.textureRect.left = jsonItemData.at("texture-x");
        if (jsonItemData.contains("texture-y")) itemData.textureRect.top = jsonItemData.at("texture-y");
        if (jsonItemData.contains("texture-width")) itemData.textureRect.width = jsonItemData.at("texture-width");
        if (jsonItemData.contains("texture-height")) itemData.textureRect.height = jsonItemData.at("texture-height");

        // Add to item name to type map
        itemNameToTypeMap[itemData.name] = itemIndex;
        itemIndex++;

        loaded_itemData.push_back(itemData);
    }

    return true;
}

const ItemData& ItemDataLoader::getItemData(ItemType item)
{
    return loaded_itemData[item];
}

ItemType ItemDataLoader::getItemTypeFromName(const std::string& itemName)
{
    return itemNameToTypeMap[itemName];
}