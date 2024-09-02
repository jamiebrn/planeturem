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

        // Handle if places object
        // if (jsonItemData.contains("places-object"))
        // {
        //     const std::string& objectName = jsonItemData.at("places-object");
        //     itemData.placesObjectType = ObjectDataLoader::getObjectTypeFromName(objectName);
        // }
        // else
        // {
        // Load as normal item if does not place object

        if (jsonItemData.contains("texture-x")) itemData.textureRect.left = jsonItemData.at("texture-x");
        if (jsonItemData.contains("texture-y")) itemData.textureRect.top = jsonItemData.at("texture-y");
        if (jsonItemData.contains("texture-width")) itemData.textureRect.width = jsonItemData.at("texture-width");
        if (jsonItemData.contains("texture-height")) itemData.textureRect.height = jsonItemData.at("texture-height");

        if (jsonItemData.contains("places-land")) itemData.placesLand = jsonItemData.at("places-land");

        if (jsonItemData.contains("max-stack-size")) itemData.maxStackSize = jsonItemData.at("max-stack-size");

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

void ItemDataLoader::createItemFromObject(ObjectType objectType, const ObjectData& objectData)
{
    ItemData objectItemData;
    objectItemData.name = objectData.name;
    objectItemData.placesObjectType = objectType;
    objectItemData.maxStackSize = 10;

    if (objectData.mythicalItem)
    {
        // Rainbow
        objectItemData.nameColor = sf::Color(0, 0, 0);
    }

    int itemIndex = loaded_itemData.size();

    loaded_itemData.push_back(objectItemData);

    itemNameToTypeMap[objectItemData.name] = itemIndex;
}

void ItemDataLoader::createItemFromTool(const std::string& toolName, ToolType toolType)
{
    ItemData toolItemData;
    toolItemData.name = toolName;
    toolItemData.toolType = toolType;
    toolItemData.maxStackSize = 1;

    int itemIndex = loaded_itemData.size();

    loaded_itemData.push_back(toolItemData);

    itemNameToTypeMap[toolItemData.name] = itemIndex;
}

void ItemDataLoader::setItemIsMaterial(ItemType item, bool isMaterial)
{
    if (loaded_itemData.size() <= item)
        return;
    
    ItemData& itemData = loaded_itemData[item];

    itemData.isMaterial = isMaterial;
}