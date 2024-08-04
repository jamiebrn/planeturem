#include "Data/EntityDataLoader.hpp"

std::vector<EntityData> EntityDataLoader::loaded_entityData;

bool EntityDataLoader::loadData(std::string objectDataPath)
{
    std::ifstream file(objectDataPath);
    nlohmann::json data = nlohmann::json::parse(file);

    // Load data
    for (nlohmann::json::iterator iter = data.begin(); iter != data.end(); ++iter)
    {
        EntityData entityData;
        auto jsonEntityData = iter.value();

        entityData.name = jsonEntityData.at("name");

        if (jsonEntityData.contains("texture-x")) entityData.textureRect.left = jsonEntityData.at("texture-x");
        if (jsonEntityData.contains("texture-y")) entityData.textureRect.top = jsonEntityData.at("texture-y");
        if (jsonEntityData.contains("texture-width")) entityData.textureRect.width = jsonEntityData.at("texture-width");
        if (jsonEntityData.contains("texture-height")) entityData.textureRect.height = jsonEntityData.at("texture-height");
        if (jsonEntityData.contains("texture-x-origin")) entityData.textureOrigin.x = jsonEntityData.at("texture-x-origin");
        if (jsonEntityData.contains("texture-y-origin")) entityData.textureOrigin.y = jsonEntityData.at("texture-y-origin");
        if (jsonEntityData.contains("health")) entityData.health = jsonEntityData.at("health");
        if (jsonEntityData.contains("size-x")) entityData.size.x = jsonEntityData.at("size-x");
        if (jsonEntityData.contains("size-y")) entityData.size.y = jsonEntityData.at("size-y");

        if (jsonEntityData.contains("item-drops"))
        {
            auto itemDrops = jsonEntityData.at("item-drops");
            for (nlohmann::json::iterator itemDropsIter = itemDrops.begin(); itemDropsIter != itemDrops.end(); ++itemDropsIter)
            {
                entityData.itemDrops[std::stoi(itemDropsIter.key())] = itemDropsIter.value();
            }
        }

        loaded_entityData.push_back(entityData);
    }

    return true;
}

const EntityData& EntityDataLoader::getEntityData(unsigned int type_index)
{    
    return loaded_entityData[type_index];
}