#include "Data/ObjectDataLoader.hpp"

std::vector<ObjectData> ObjectDataLoader::loaded_objectData;

bool ObjectDataLoader::loadData(std::string objectDataPath)
{
    std::ifstream file(objectDataPath);
    nlohmann::json data = nlohmann::json::parse(file);

    // Load data
    for (nlohmann::json::iterator iter = data.begin(); iter != data.end(); ++iter)
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
        if (jsonObjectData.contains("draw-layer")) objectData.drawLayer = jsonObjectData.at("draw-layer");

        if (jsonObjectData.contains("item-drops"))
        {
            auto itemDrops = jsonObjectData.at("item-drops");
            for (nlohmann::json::iterator itemDropsIter = itemDrops.begin(); itemDropsIter != itemDrops.end(); ++itemDropsIter)
            {
                objectData.itemDrops[std::stoi(itemDropsIter.key())] = itemDropsIter.value();
            }
        }

        loaded_objectData.push_back(objectData);
    }

    return true;
}

const ObjectData& ObjectDataLoader::getObjectData(int index)
{    
    return loaded_objectData[index];
}