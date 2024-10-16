#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <string>
#include <unordered_map>

#include "Core/json.hpp"
#include "Data/Serialise/IntRectSerialise.hpp"
#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ArmourData.hpp"

class ItemDataLoader
{
    ItemDataLoader() = delete;

public:
    static bool loadData(std::string itemDataPath);

    static const ItemData& getItemData(ItemType item);

    static ItemType getItemTypeFromName(const std::string& itemName);

    static void createItemFromObject(ObjectType objectType, const ObjectData& objectData);

    static void createItemFromTool(const std::string& toolName, ToolType toolType);

    static void createItemFromArmour(ArmourType armourType, const ArmourData& armourData);

    static void setItemIsMaterial(ItemType item, bool isMaterial = true);

private:
    static std::vector<ItemData> loaded_itemData;

    static std::unordered_map<std::string, ItemType> itemNameToTypeMap;

};