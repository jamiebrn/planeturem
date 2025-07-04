#include "Data/ItemDataLoader.hpp"

std::vector<ItemData> ItemDataLoader::loaded_itemData;
std::unordered_map<std::string, ItemType> ItemDataLoader::itemNameToTypeMap;
std::vector<ItemType> ItemDataLoader::currencyItemOrder;
std::unordered_map<std::string, std::unordered_map<int, std::vector<ItemType>>> ItemDataLoader::craftingStationItemMap;

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

        if (jsonItemData.contains("consumable-data"))
        {
            auto jsonConsumableData = jsonItemData.at("consumable-data");

            ConsumableData consumableData;
            if (jsonConsumableData.contains("cooldown-time")) consumableData.cooldownTime = jsonConsumableData.at("cooldown-time");
            
            if (jsonConsumableData.contains("health-increase"))
            {
                consumableData.healthIncrease = jsonConsumableData.at("health-increase");
            }

            if (jsonConsumableData.contains("permanent-health-increase"))
            {
                consumableData.permanentHealthIncrease = jsonConsumableData.at("permanent-health-increase");
            }

            itemData.consumableData = consumableData;
        }

        if (jsonItemData.contains("max-stack-size")) itemData.maxStackSize = jsonItemData.at("max-stack-size");

        if (jsonItemData.contains("currency-value")) itemData.currencyValue = jsonItemData.at("currency-value");

        if (jsonItemData.contains("sell-value")) itemData.sellValue = jsonItemData.at("sell-value");

        if (jsonItemData.contains("name-colour")) itemData.nameColor = jsonItemData.at("name-colour");

        if (jsonItemData.contains("achievement-unlock-on-obtain")) itemData.achievementUnlockOnObtain = jsonItemData.at("achievement-unlock-on-obtain");

        // Add to item name to type map
        itemNameToTypeMap[itemData.name] = itemIndex;
        itemIndex++;

        loaded_itemData.push_back(itemData);
    }

    createCurrencyItemOrderVector();

    return true;
}

void ItemDataLoader::createCurrencyItemOrderVector()
{
    currencyItemOrder.clear();

    for (int i = 0; i < loaded_itemData.size(); i++)
    {
        if (loaded_itemData[i].currencyValue <= 0)
        {
            continue;
        }

        currencyItemOrder.push_back(i);
    }

    // Sort
    std::sort(currencyItemOrder.begin(), currencyItemOrder.end(), [](ItemType a, ItemType b)
    {
        return getItemData(a).currencyValue > getItemData(b).currencyValue;
    });
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

void ItemDataLoader::createItemFromObject(ObjectType objectType, const ObjectData& objectData, float sellValue, std::optional<std::string> displayName,
    std::string achievementUnlockOnObtain)
{
    ItemData objectItemData;
    objectItemData.name = objectData.name;
    objectItemData.displayName = displayName;
    objectItemData.placesObjectType = objectType;
    objectItemData.maxStackSize = 50;
    objectItemData.sellValue = sellValue;
    objectItemData.achievementUnlockOnObtain = achievementUnlockOnObtain;

    if (objectData.mythicalItem)
    {
        // Rainbow
        objectItemData.nameColor = pl::Color(0, 0, 0);
    }

    int itemIndex = loaded_itemData.size();

    if (!objectData.craftingStation.empty())
    {
        craftingStationItemMap[objectData.craftingStation][objectData.craftingStationLevel].push_back(itemIndex);
    }

    loaded_itemData.push_back(objectItemData);

    itemNameToTypeMap[objectItemData.name] = itemIndex;
}

void ItemDataLoader::createItemFromTool(const std::string& toolName, ToolType toolType, float sellValue,
    std::string achievementUnlockOnObtain)
{
    ItemData toolItemData;
    toolItemData.name = toolName;
    toolItemData.toolType = toolType;
    toolItemData.maxStackSize = 1;
    toolItemData.sellValue = sellValue;
    toolItemData.achievementUnlockOnObtain = achievementUnlockOnObtain;

    int itemIndex = loaded_itemData.size();

    loaded_itemData.push_back(toolItemData);

    itemNameToTypeMap[toolItemData.name] = itemIndex;
}

void ItemDataLoader::createItemFromArmour(ArmourType armourType, const ArmourData& armourData, float sellValue,
    std::string achievementUnlockOnObtain)
{
    ItemData armourItemData;
    armourItemData.name = armourData.name;
    armourItemData.armourType = armourType;
    armourItemData.maxStackSize = 1;
    armourItemData.sellValue = sellValue;
    armourItemData.achievementUnlockOnObtain = achievementUnlockOnObtain;

    int itemIndex = loaded_itemData.size();

    loaded_itemData.push_back(armourItemData);

    itemNameToTypeMap[armourItemData.name] = itemIndex;
}

ItemType ItemDataLoader::createItemFromProjectile(ProjectileType projectileType, const ProjectileData& projectileData, float sellValue,
    std::string achievementUnlockOnObtain)
{
    ItemData projectileItemData;
    projectileItemData.name = projectileData.name;
    projectileItemData.projectileType = projectileType;
    projectileItemData.maxStackSize = 99;
    projectileItemData.sellValue = sellValue;
    projectileItemData.achievementUnlockOnObtain = achievementUnlockOnObtain;

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

const std::vector<ItemType>& ItemDataLoader::getCraftingStationLevelItems(const std::string& craftingStationName, int craftingStationLevel)
{
    assert(craftingStationItemMap.contains(craftingStationName));
    assert(craftingStationItemMap.at(craftingStationName).contains(craftingStationLevel));

    return (craftingStationItemMap.at(craftingStationName).at(craftingStationLevel));
}