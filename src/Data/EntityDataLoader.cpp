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

        int textureWidth = jsonEntityData.at("texture-width");
        int textureHeight = jsonEntityData.at("texture-height");

        if (jsonEntityData.contains("texture-idle"))
        {
            auto idleTexture = jsonEntityData.at("texture-idle");
            for (nlohmann::json::iterator texturePositionIter = idleTexture.begin(); texturePositionIter != idleTexture.end(); ++texturePositionIter)
            {
                sf::IntRect textureRect;
                textureRect.left = texturePositionIter.value()[0];
                textureRect.top = texturePositionIter.value()[1];
                textureRect.width = textureWidth;
                textureRect.height = textureHeight;

                entityData.idleTextureRects.push_back(textureRect);
            }
        }

        if (jsonEntityData.contains("texture-walk-speed")) entityData.walkAnimSpeed = jsonEntityData.at("texture-walk-speed");

        if (jsonEntityData.contains("texture-walk"))
        {
            auto walkTexture = jsonEntityData.at("texture-walk");
            for (nlohmann::json::iterator texturePositionIter = walkTexture.begin(); texturePositionIter != walkTexture.end(); ++texturePositionIter)
            {
                sf::IntRect textureRect;
                textureRect.left = texturePositionIter.value()[0];
                textureRect.top = texturePositionIter.value()[1];
                textureRect.width = textureWidth;
                textureRect.height = textureHeight;

                entityData.walkTextureRects.push_back(textureRect);
            }
        }

        if (jsonEntityData.contains("texture-idle-speed")) entityData.idleAnimSpeed = jsonEntityData.at("texture-idle-speed");

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

const EntityData& EntityDataLoader::getEntityData(EntityType entity)
{    
    return loaded_entityData[entity];
}