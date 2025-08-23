#include "Data/EntityDataLoader.hpp"
#include <extlib/hashpp.h>

std::vector<EntityData> EntityDataLoader::loaded_entityData;
std::unordered_map<std::string, EntityType> EntityDataLoader::entityNameToTypeMap;

std::string EntityDataLoader::dataHash;

bool EntityDataLoader::loadData(std::string objectDataPath)
{
    std::ifstream file(objectDataPath);
    nlohmann::json data = nlohmann::json::parse(file);

    int entityIdx = 0;

    // Load data
    for (nlohmann::json::iterator iter = data.begin(); iter != data.end(); ++iter)
    {
        EntityData entityData;
        auto jsonEntityData = iter.value();

        entityData.name = jsonEntityData.at("name");

        pl::Vector2f textureSize = jsonEntityData.at("texture-size");

        if (jsonEntityData.contains("texture-idle"))
        {
            auto idleTexture = jsonEntityData.at("texture-idle");
            for (nlohmann::json::iterator texturePositionIter = idleTexture.begin(); texturePositionIter != idleTexture.end(); ++texturePositionIter)
            {
                pl::Rect<int> textureRect;
                textureRect.x = texturePositionIter.value()[0];
                textureRect.y = texturePositionIter.value()[1];
                textureRect.width = textureSize.x;
                textureRect.height = textureSize.y;

                entityData.idleTextureRects.push_back(textureRect);
            }
        }

        if (jsonEntityData.contains("texture-walk-speed")) entityData.walkAnimSpeed = jsonEntityData.at("texture-walk-speed");

        if (jsonEntityData.contains("texture-walk"))
        {
            auto walkTexture = jsonEntityData.at("texture-walk");
            for (nlohmann::json::iterator texturePositionIter = walkTexture.begin(); texturePositionIter != walkTexture.end(); ++texturePositionIter)
            {
                pl::Rect<int> textureRect;
                textureRect.x = texturePositionIter.value()[0];
                textureRect.y = texturePositionIter.value()[1];
                textureRect.width = textureSize.x;
                textureRect.height = textureSize.y;

                entityData.walkTextureRects.push_back(textureRect);
            }
        }

        if (jsonEntityData.contains("texture-idle-speed")) entityData.idleAnimSpeed = jsonEntityData.at("texture-idle-speed");

        if (jsonEntityData.contains("texture-origin")) entityData.textureOrigin = jsonEntityData.at("texture-origin");
        if (jsonEntityData.contains("health")) entityData.health = jsonEntityData.at("health");
        if (jsonEntityData.contains("size")) entityData.size = jsonEntityData.at("size");

        if (jsonEntityData.contains("hit-collision")) entityData.hitCollision = jsonEntityData.at("hit-collision");
        if (jsonEntityData.contains("damage")) entityData.damage = jsonEntityData.at("damage");

        if (jsonEntityData.contains("item-drops"))
        {
            auto itemDrops = jsonEntityData.at("item-drops");
            for (nlohmann::json::iterator itemDropsIter = itemDrops.begin(); itemDropsIter != itemDrops.end(); ++itemDropsIter)
            {
                ItemDrop itemDrop;
                itemDrop.item = ItemDataLoader::getItemTypeFromName(itemDropsIter.key());
                itemDrop.minAmount = itemDropsIter.value()[0];
                itemDrop.maxAmount = itemDropsIter.value()[1];
                itemDrop.chance = itemDropsIter.value()[2];

                entityData.itemDrops.push_back(itemDrop);
            }
        }

        entityData.behaviour = jsonEntityData.at("behaviour");

        if (jsonEntityData.contains("behaviour-parameters"))
        {
            entityData.behaviourParameters = jsonEntityData.at("behaviour-parameters");
        }

        loaded_entityData.push_back(entityData);

        entityNameToTypeMap[entityData.name] = entityIdx;

        entityIdx++;
    }

    dataHash = hashpp::get::getFileHash(hashpp::ALGORITHMS::MD5, objectDataPath);

    return true;
}

const EntityData& EntityDataLoader::getEntityData(EntityType entity)
{    
    return loaded_entityData[entity];
}

EntityType EntityDataLoader::getEntityTypeFromName(const std::string& entityName)
{
    return entityNameToTypeMap[entityName];
}