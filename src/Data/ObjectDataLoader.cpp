#include "Data/ObjectDataLoader.hpp"

std::vector<ObjectData> ObjectDataLoader::loaded_objectData;

bool ObjectDataLoader::loadData(std::string objectDataPath)
{
    std::ifstream file(objectDataPath);
    nlohmann::ordered_json data = nlohmann::ordered_json::parse(file);

    // Load data
    for (nlohmann::ordered_json::iterator iter = data.begin(); iter != data.end(); ++iter)
    {
        ObjectData objectData;
        auto jsonObjectData = iter.value();

        objectData.name = jsonObjectData.at("name");

        if (jsonObjectData.contains("texture-x")) objectData.textureRect.left = jsonObjectData.at("texture-x");
        if (jsonObjectData.contains("texture-y")) objectData.textureRect.top = jsonObjectData.at("texture-y");
        if (jsonObjectData.contains("texture-width")) objectData.textureRect.width = jsonObjectData.at("texture-width");
        if (jsonObjectData.contains("texture-height")) objectData.textureRect.height = jsonObjectData.at("texture-height");
        if (jsonObjectData.contains("texture-x-origin")) objectData.textureOrigin.x = jsonObjectData.at("texture-x-origin");
        if (jsonObjectData.contains("texture-y-origin")) objectData.textureOrigin.y = jsonObjectData.at("texture-y-origin");
        if (jsonObjectData.contains("health")) objectData.health = jsonObjectData.at("health");
        if (jsonObjectData.contains("has-collision")) objectData.hasCollision = jsonObjectData.at("has-collision");
        if (jsonObjectData.contains("place-on-water")) objectData.placeOnWater = jsonObjectData.at("place-on-water");
        if (jsonObjectData.contains("draw-layer")) objectData.drawLayer = jsonObjectData.at("draw-layer");
        if (jsonObjectData.contains("size-x")) objectData.size.x = jsonObjectData.at("size-x");
        if (jsonObjectData.contains("size-y")) objectData.size.y = jsonObjectData.at("size-y");
        if (jsonObjectData.contains("crafting-station")) objectData.craftingStation = jsonObjectData.at("crafting-station");
        if (jsonObjectData.contains("crafting-station-level")) objectData.craftingStationLevel = jsonObjectData.at("crafting-station-level");

        if (jsonObjectData.contains("item-drops"))
        {
            auto itemDrops = jsonObjectData.at("item-drops");
            for (nlohmann::ordered_json::iterator itemDropsIter = itemDrops.begin(); itemDropsIter != itemDrops.end(); ++itemDropsIter)
            {
                ItemDrop itemDrop;
                itemDrop.item = std::stoi(itemDropsIter.key());
                itemDrop.minAmount = itemDropsIter.value()[0];
                itemDrop.maxAmount = itemDropsIter.value()[1];
                itemDrop.chance = itemDropsIter.value()[2];

                objectData.itemDrops.push_back(itemDrop);
            }
        }

        loaded_objectData.push_back(objectData);
    }

    return true;
}

const ObjectData& ObjectDataLoader::getObjectData(ObjectType type_index)
{    
    return loaded_objectData[type_index];
}