#pragma once

// #include <SFML/Graphics.hpp>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/json.hpp"
#include "Data/Serialise/IntRectSerialise.hpp"
#include "Data/Serialise/ColorSerialise.hpp"
#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ToolData.hpp"
#include "Data/ArmourData.hpp"

class ItemDataLoader
{
    ItemDataLoader() = delete;

public:
    static bool loadData(std::string itemDataPath);

    static const ItemData& getItemData(ItemType item);

    static ItemType getItemTypeFromName(const std::string& itemName);

    static void createItemFromObject(ObjectType objectType, const ObjectData& objectData, float sellValue, std::optional<std::string> displayName);

    static void createItemFromTool(const std::string& toolName, ToolType toolType, float sellValue);

    static void createItemFromArmour(ArmourType armourType, const ArmourData& armourData, float sellValue);

    // Returns the item type corresponding to the added projectile
    static ItemType createItemFromProjectile(ProjectileType projectileType, const ProjectileData& projectileData, float sellValue);

    static void setItemIsMaterial(ItemType item, bool isMaterial = true);

    static const std::vector<ItemType>& getCraftingStationLevelItems(const std::string& craftingStationName, int craftingStationLevel);

    inline static const std::unordered_map<std::string, ItemType>& getItemNameToTypeMap() {return itemNameToTypeMap;}

    // Highest value is at lowest index
    inline static const std::vector<ItemType>& getCurrencyItemOrderVector() {return currencyItemOrder;}

private:
    static void createCurrencyItemOrderVector();

private:
    static std::vector<ItemData> loaded_itemData;

    static std::unordered_map<std::string, ItemType> itemNameToTypeMap;

    static std::vector<ItemType> currencyItemOrder;

    static std::unordered_map<std::string, std::unordered_map<int, std::vector<ItemType>>> craftingStationItemMap;

};