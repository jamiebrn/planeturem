#include "Data/ObjectDataLoader.hpp"

std::vector<ObjectData> ObjectDataLoader::loaded_objectData;
std::unordered_map<std::string, ObjectType> ObjectDataLoader::objectNameToTypeMap;

bool ObjectDataLoader::loadData(std::string objectDataPath)
{
    std::ifstream file(objectDataPath);
    nlohmann::ordered_json data = nlohmann::ordered_json::parse(file);

    int objectIdx = 0;

    // Store all item drops to add to objects after loading
    // Allows objects to drop other objects, including themselves, as items
    // Pair stores name of item to drop as string, and item drop (item drop item type will be overridden)
    std::unordered_map<ObjectType, std::vector<std::pair<std::string, ItemDrop>>> objectItemDrops;

    // Load data
    for (nlohmann::ordered_json::iterator iter = data.begin(); iter != data.end(); ++iter)
    {
        ObjectData objectData;
        auto jsonObjectData = iter.value();

        objectData.name = jsonObjectData.at("name");

        int textureWidth = jsonObjectData.at("texture-width");
        int textureHeight = jsonObjectData.at("texture-height");

        auto textures = jsonObjectData.at("textures");
        for (nlohmann::ordered_json::iterator texturePositionIter = textures.begin(); texturePositionIter != textures.end(); ++texturePositionIter)
        {
            sf::IntRect textureRect;
            textureRect.left = texturePositionIter.value()[0];
            textureRect.top = texturePositionIter.value()[1];
            textureRect.width = textureWidth;
            textureRect.height = textureHeight;

            objectData.textureRects.push_back(textureRect);
        }

        if (jsonObjectData.contains("texture-frame-delay")) objectData.textureFrameDelay = jsonObjectData.at("texture-frame-delay");

        if (jsonObjectData.contains("texture-x-origin")) objectData.textureOrigin.x = jsonObjectData.at("texture-x-origin");
        if (jsonObjectData.contains("texture-y-origin")) objectData.textureOrigin.y = jsonObjectData.at("texture-y-origin");
        if (jsonObjectData.contains("health")) objectData.health = jsonObjectData.at("health");

        if (jsonObjectData.contains("light-emission")) 
        {
            auto lightEmissionFrames = jsonObjectData.at("light-emission");
            for (auto iter = lightEmissionFrames.begin(); iter != lightEmissionFrames.end(); ++iter)
            {
                objectData.lightEmissionFrames.push_back(iter.value());
            }
        }

        if (jsonObjectData.contains("light-absorption")) objectData.lightAbsorption = jsonObjectData.at("light-absorption");
        
        if (jsonObjectData.contains("has-collision")) objectData.hasCollision = jsonObjectData.at("has-collision");
        if (jsonObjectData.contains("place-on-water")) objectData.placeOnWater = jsonObjectData.at("place-on-water");
        if (jsonObjectData.contains("draw-layer")) objectData.drawLayer = jsonObjectData.at("draw-layer");
        if (jsonObjectData.contains("size-x")) objectData.size.x = jsonObjectData.at("size-x");
        if (jsonObjectData.contains("size-y")) objectData.size.y = jsonObjectData.at("size-y");

        if (jsonObjectData.contains("crafting-station")) objectData.craftingStation = jsonObjectData.at("crafting-station");
        if (jsonObjectData.contains("crafting-station-level")) objectData.craftingStationLevel = jsonObjectData.at("crafting-station-level");

        if (jsonObjectData.contains("chest-capacity")) objectData.chestCapacity = jsonObjectData.at("chest-capacity");

        if (jsonObjectData.contains("mythical-item")) objectData.mythicalItem = jsonObjectData.at("mythical-item");

        if (jsonObjectData.contains("item-drops"))
        {
            auto itemDrops = jsonObjectData.at("item-drops");
            for (nlohmann::ordered_json::iterator itemDropsIter = itemDrops.begin(); itemDropsIter != itemDrops.end(); ++itemDropsIter)
            {
                ItemDrop itemDrop;
                // itemDrop.item = ItemDataLoader::getItemTypeFromName(itemDropsIter.key());
                itemDrop.minAmount = itemDropsIter.value()[0];
                itemDrop.maxAmount = itemDropsIter.value()[1];
                itemDrop.chance = itemDropsIter.value()[2];

                // objectData.itemDrops.push_back(itemDrop);
                objectItemDrops[objectIdx].push_back({itemDropsIter.key(), itemDrop});
            }
        }

        if (jsonObjectData.contains("rocket-info"))
        {
            auto rocketInfo = jsonObjectData.at("rocket-info");
            objectData.rocketObjectData = RocketObjectData();
            
            auto textureRect = rocketInfo.at("texture");
            auto textureOrigin = rocketInfo.at("texture-origin");
            auto launchPosition = rocketInfo.at("launch-position");
            auto availableDestinations = rocketInfo.at("available-destinations");
            objectData.rocketObjectData->textureRect = sf::IntRect(textureRect[0], textureRect[1], textureRect[2], textureRect[3]);
            objectData.rocketObjectData->textureOrigin = sf::Vector2f(textureOrigin[0], textureOrigin[1]);
            objectData.rocketObjectData->launchPosition = sf::Vector2f(launchPosition[0], launchPosition[1]);

            for (nlohmann::ordered_json::iterator destinationIter = availableDestinations.begin(); destinationIter != availableDestinations.end(); ++destinationIter)
            {
                objectData.rocketObjectData->avaiableDestinationStrings.push_back(destinationIter.value());
            }
        }

        loaded_objectData.push_back(objectData);

        objectNameToTypeMap[objectData.name] = objectIdx;

        // Create item corresponding to object
        ItemDataLoader::createItemFromObject(objectIdx, objectData);

        objectIdx++;
    }

    // Load all item drops into objects
    for (const auto& objectItemDrop : objectItemDrops)
    {
        const ObjectType& objectType = objectItemDrop.first;
        const std::vector<std::pair<std::string, ItemDrop>>& itemDrops = objectItemDrop.second;

        for (const auto& itemDropWithString : itemDrops)
        {
            ItemDrop itemDrop = itemDropWithString.second;
            itemDrop.item = ItemDataLoader::getItemTypeFromName(itemDropWithString.first);

            loaded_objectData[objectType].itemDrops.push_back(itemDrop);
        }
    }

    return true;
}

bool ObjectDataLoader::loadRocketPlanetDestinations(const std::unordered_map<std::string, PlanetType>& planetStringToTypeMap)
{
    // Load rocket destinations for all rocket objects
    for (ObjectData& objectData : loaded_objectData)
    {
        // If not rocket, do not load data
        if (!objectData.rocketObjectData.has_value())
            continue;
        
        // Load planet destinations from strings
        for (const std::string& planetStr : objectData.rocketObjectData->avaiableDestinationStrings)
        {
            if (!planetStringToTypeMap.contains(planetStr))
                return false;

            PlanetType planetType = planetStringToTypeMap.at(planetStr);
            objectData.rocketObjectData->availableDestinations.push_back(planetType);
        }

        // Free planet string destinations, as only used temporarily for loading
        objectData.rocketObjectData->avaiableDestinationStrings.clear();
    }

    return true;
}

const ObjectData& ObjectDataLoader::getObjectData(ObjectType type_index)
{    
    return loaded_objectData[type_index];
}

ObjectType ObjectDataLoader::getObjectTypeFromName(const std::string& objectName)
{
    return objectNameToTypeMap[objectName];
}