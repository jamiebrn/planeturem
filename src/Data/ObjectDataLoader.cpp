#include "Data/ObjectDataLoader.hpp"
#include "Player/ShopInventoryData.hpp"

std::vector<ObjectData> ObjectDataLoader::loaded_objectData;
std::unordered_map<std::string, ObjectType> ObjectDataLoader::objectNameToTypeMap;

bool ObjectDataLoader::loadData(std::string objectDataPath)
{
    std::ifstream file(objectDataPath);
    nlohmann::ordered_json data = nlohmann::ordered_json::parse(file);

    int objectIdx = 0;

    // Load all names and essential data first to load items, to allow objects to drop other objects / themselves
    for (auto iter = data.begin(); iter != data.end(); ++iter)
    {
        ObjectData minimalObjectData;

        minimalObjectData.name = iter.value().at("name");
        if (iter.value().contains("mythical-item")) minimalObjectData.mythicalItem = iter.value().at("mythical-item");

        objectNameToTypeMap[minimalObjectData.name] = objectIdx;

        float sellValue = 0;
        if (iter.value().contains("sell-value")) sellValue = iter.value().at("sell-value");

        if (iter.value().contains("crafting-station")) minimalObjectData.craftingStation = iter.value().at("crafting-station");
        if (iter.value().contains("crafting-station-level")) minimalObjectData.craftingStationLevel = iter.value().at("crafting-station-level");

        ItemDataLoader::createItemFromObject(objectIdx, minimalObjectData, sellValue);

        objectIdx++;
    }

    // objectIdx = 0;

    // Load data
    for (nlohmann::ordered_json::iterator iter = data.begin(); iter != data.end(); ++iter)
    {
        ObjectData objectData;
        auto jsonObjectData = iter.value();

        objectData.name = jsonObjectData.at("name");

        if (jsonObjectData.contains("textures"))
        {
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
                itemDrop.item = ItemDataLoader::getItemTypeFromName(itemDropsIter.key());
                itemDrop.minAmount = itemDropsIter.value()[0];
                itemDrop.maxAmount = itemDropsIter.value()[1];
                itemDrop.chance = itemDropsIter.value()[2];

                objectData.itemDrops.push_back(itemDrop);
                // objectItemDrops[objectIdx].push_back({itemDropsIter.key(), itemDrop});
            }
        }

        // Load rocket data if is rocket
        if (jsonObjectData.contains("rocket-info"))
        {
            auto rocketInfo = jsonObjectData.at("rocket-info");
            objectData.rocketObjectData = RocketObjectData();
            
            auto textureRect = rocketInfo.at("texture");
            auto textureOrigin = rocketInfo.at("texture-origin");
            auto launchPosition = rocketInfo.at("launch-position");
            auto availableDestinations = rocketInfo.at("available-destinations");
            auto availableRoomDestinations = rocketInfo.at("available-room-destinations");

            objectData.rocketObjectData->textureRect = sf::IntRect(textureRect[0], textureRect[1], textureRect[2], textureRect[3]);
            objectData.rocketObjectData->textureOrigin = sf::Vector2f(textureOrigin[0], textureOrigin[1]);
            objectData.rocketObjectData->launchPosition = sf::Vector2f(launchPosition[0], launchPosition[1]);

            for (nlohmann::ordered_json::iterator destinationIter = availableDestinations.begin(); destinationIter != availableDestinations.end(); ++destinationIter)
            {
                objectData.rocketObjectData->availableDestinationStrings.push_back(destinationIter.value());
            }

            for (nlohmann::ordered_json::iterator roomDestinationIter = availableRoomDestinations.begin(); roomDestinationIter != availableRoomDestinations.end();
                ++roomDestinationIter)
            {
                objectData.rocketObjectData->availableRoomDestinationStrings.push_back(roomDestinationIter.value());
            }
        }

        // Load plant data if is plant
        if (jsonObjectData.contains("plant-data"))
        {
            auto plantData = jsonObjectData.at("plant-data");
            auto plantStages = plantData.at("stages");
            objectData.plantStageObjectData = std::vector<PlantStageObjectData>();

            // Load all stages
            for (auto stageIter = plantStages.begin(); stageIter != plantStages.end(); ++stageIter)
            {
                PlantStageObjectData stageData;
                
                sf::Vector2i textureSize = stageIter.value().at("texture-size");
                auto textures = stageIter.value().at("textures");
                for (auto textureIter = textures.begin(); textureIter != textures.end(); ++textureIter)
                {
                    sf::IntRect textureRect;
                    textureRect.left = textureIter.value()[0];
                    textureRect.top = textureIter.value()[1];
                    textureRect.width = textureSize.x;
                    textureRect.height = textureSize.y;
                    stageData.textureRects.push_back(textureRect);
                }

                stageData.textureOrigin = stageIter.value().at("texture-origin");
                stageData.health = stageIter.value().at("health");
                
                if (stageIter.value().contains("day-range"))
                {
                    auto dayRange = stageIter.value().at("day-range");
                    stageData.minDay = dayRange[0];
                    stageData.maxDay = dayRange[1];
                }

                if (stageIter.value().contains("item-drops"))
                {
                    auto itemDrops = stageIter.value().at("item-drops");
                    for (auto itemDropsIter = itemDrops.begin(); itemDropsIter != itemDrops.end(); ++itemDropsIter)
                    {
                        ItemDrop itemDrop;
                        itemDrop.item = ItemDataLoader::getItemTypeFromName(itemDropsIter.key());
                        itemDrop.minAmount = itemDropsIter.value()[0];
                        itemDrop.maxAmount = itemDropsIter.value()[1];
                        itemDrop.chance = itemDropsIter.value()[2];

                        stageData.itemDrops.push_back(itemDrop);
                    }
                }

                objectData.plantStageObjectData->push_back(stageData);
            }
        }

        // Load data if is NPC
        if (jsonObjectData.contains("npc-data"))
        {
            NPCObjectData npcObjectData;
            auto jsonNpcData = jsonObjectData.at("npc-data");

            npcObjectData.npcName = jsonNpcData.at("npc-name");

            static const std::unordered_map<std::string, NPCObjectBehaviour> stringToNPCBehaviourMap = {
                {"none", NPCObjectBehaviour::None},
                {"talk", NPCObjectBehaviour::Talk},
                {"shop", NPCObjectBehaviour::Shop}
            };

            npcObjectData.behaviour = stringToNPCBehaviourMap.at(jsonNpcData.at("behaviour"));

            npcObjectData.portraitTextureOffset = jsonNpcData.at("portrait-texture-offset");

            npcObjectData.dialogueLines = jsonNpcData.at("dialogue");

            if (jsonNpcData.contains("shop-items"))
            {
                for (std::pair<std::string, int> itemCount : jsonNpcData.at("shop-items"))
                {
                    npcObjectData.shopItems.push_back({ItemDataLoader::getItemTypeFromName(itemCount.first), itemCount.second});
                }

                if (jsonNpcData.contains("buy-prices"))
                {
                    std::unordered_map<std::string, float> buyPricesStrings = jsonNpcData.at("buy-prices");
                    for (auto iter = buyPricesStrings.begin(); iter != buyPricesStrings.end(); ++iter)
                    {
                        npcObjectData.buyPriceMults[ItemDataLoader::getItemTypeFromName(iter->first)] = iter->second;
                    }
                }

                if (jsonNpcData.contains("sell-prices"))
                {
                    std::unordered_map<std::string, float> sellPricesStrings = jsonNpcData.at("sell-prices");
                    for (auto iter = sellPricesStrings.begin(); iter != sellPricesStrings.end(); ++iter)
                    {
                        npcObjectData.sellPriceMults[ItemDataLoader::getItemTypeFromName(iter->first)] = iter->second;
                    }
                }
            }

            objectData.npcObjectData = npcObjectData;
        }
        
        // Load data if is landmark
        if (jsonObjectData.contains("is-landmark"))
        {
            objectData.isLandmark = jsonObjectData.at("is-landmark");
        }

        loaded_objectData.push_back(objectData);
    }

    return true;
}

bool ObjectDataLoader::loadRocketPlanetDestinations(const std::unordered_map<std::string, PlanetType>& planetStringToTypeMap,
    const std::unordered_map<std::string, RoomType>& roomStringToTypeMap)
{
    // Load rocket destinations for all rocket objects
    for (ObjectData& objectData : loaded_objectData)
    {
        // If not rocket, do not load data
        if (!objectData.rocketObjectData.has_value())
            continue;
        
        // Load planet destinations from strings
        for (const std::string& planetStr : objectData.rocketObjectData->availableDestinationStrings)
        {
            if (!planetStringToTypeMap.contains(planetStr))
                return false;

            PlanetType planetType = planetStringToTypeMap.at(planetStr);
            objectData.rocketObjectData->availableDestinations.push_back(planetType);
        }

        // Load room destinations
        for (const std::string& roomStr : objectData.rocketObjectData->availableRoomDestinationStrings)
        {
            if (!roomStringToTypeMap.contains(roomStr))
                return false;

            RoomType roomType = roomStringToTypeMap.at(roomStr);
            objectData.rocketObjectData->availableRoomDestinations.push_back(roomType);
        }

        // Free planet string destinations, as only used temporarily for loading
        objectData.rocketObjectData->availableDestinationStrings.clear();
    }

    return true;
}

const ObjectData& ObjectDataLoader::getObjectData(ObjectType objectType)
{    
    return loaded_objectData[objectType];
}

ObjectType ObjectDataLoader::getObjectTypeFromName(const std::string& objectName)
{
    if (!objectNameToTypeMap.contains(objectName))
    {
        return 0;
    }

    return objectNameToTypeMap[objectName];
}