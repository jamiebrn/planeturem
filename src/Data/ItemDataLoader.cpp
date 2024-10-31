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

        if (jsonItemData.contains("description")) itemData.description = jsonItemData.at("description");

        itemData.textureRect = jsonItemData.at("texture");

        if (jsonItemData.contains("places-land")) itemData.placesLand = jsonItemData.at("places-land");

        if (jsonItemData.contains("boss-summon-data"))
        {
            auto jsonBossSummonData = jsonItemData.at("boss-summon-data");
            
            BossSummonData bossSummonData;
            bossSummonData.bossName = jsonBossSummonData.at("boss-name");
            if (jsonBossSummonData.contains("use-at-night")) bossSummonData.useAtNight = jsonBossSummonData.at("use-at-night");

            itemData.bossSummonData = bossSummonData;
        }

        if (jsonItemData.contains("max-stack-size")) itemData.maxStackSize = jsonItemData.at("max-stack-size");

        if (jsonItemData.contains("currency-value")) itemData.currencyValue = jsonItemData.at("currency-value");

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
    if (!itemNameToTypeMap.contains(itemName))
    {
        return 0;
    }

    return itemNameToTypeMap[itemName];
}

void ItemDataLoader::createItemFromObject(ObjectType objectType, const ObjectData& objectData)
{
    ItemData objectItemData;
    objectItemData.name = objectData.name;
    objectItemData.placesObjectType = objectType;
    objectItemData.maxStackSize = 50;

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

void ItemDataLoader::createItemFromArmour(ArmourType armourType, const ArmourData& armourData)
{
    ItemData armourItemData;
    armourItemData.name = armourData.name;
    armourItemData.armourType = armourType;
    armourItemData.maxStackSize = 1;

    int itemIndex = loaded_itemData.size();

    loaded_itemData.push_back(armourItemData);

    itemNameToTypeMap[armourItemData.name] = itemIndex;
}

ItemType ItemDataLoader::createItemFromProjectile(ProjectileType projectileType, const ProjectileData& projectileData)
{
    ItemData projectileItemData;
    projectileItemData.name = projectileData.name;
    projectileItemData.projectileType = projectileType;
    projectileItemData.maxStackSize = 99;

    int itemIndex = loaded_itemData.size();

    loaded_itemData.push_back(projectileItemData);

    itemNameToTypeMap[projectileItemData.name] = itemIndex;

    return itemIndex;
}

void ItemDataLoader::setItemIsMaterial(ItemType item, bool isMaterial)
{
    if (loaded_itemData.size() <= item)
        return;
    
    ItemData& itemData = loaded_itemData[item];

    itemData.isMaterial = isMaterial;
}