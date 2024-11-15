#include "Data/StructureDataLoader.hpp"

std::vector<StructureData> StructureDataLoader::loaded_structureData;
std::vector<RoomData> StructureDataLoader::loaded_roomData;

std::unordered_map<std::string, StructureType> StructureDataLoader::structureNameToTypeMap;
std::unordered_map<std::string, RoomType> StructureDataLoader::roomTypeTravelLocationsMap;

bool StructureDataLoader::loadData(std::string structureDataPath)
{
    std::ifstream file(structureDataPath);
    nlohmann::ordered_json data = nlohmann::ordered_json::parse(file);

    // Load rooms
    std::unordered_map<std::string, RoomType> roomNameToTypeMap;

    auto rooms = data.at("rooms");
    for (nlohmann::ordered_json::iterator iter = rooms.begin(); iter != rooms.end(); ++iter)
    {
        RoomData roomData;

        roomData.name = iter.key();

        auto tileSize = iter->at("tile-size");
        roomData.tileSize.x = tileSize[0];
        roomData.tileSize.y = tileSize[1];

        auto textureOffset = iter->at("texture");
        roomData.textureRect.left = textureOffset[0];
        roomData.textureRect.top = textureOffset[1];
        roomData.textureRect.width = roomData.tileSize.x * TILE_SIZE_PIXELS_UNSCALED;
        roomData.textureRect.height = roomData.tileSize.y * TILE_SIZE_PIXELS_UNSCALED;

        roomData.collisionBitmaskOffset = iter->at("collision-bitmask");

        if (iter->contains("objects"))
        {
            auto objects = iter->at("objects");
            for (nlohmann::ordered_json::iterator objectIter = objects.begin(); objectIter != objects.end(); ++objectIter)
            {
                RoomObjectData roomObjectData;
                roomObjectData.objectType = ObjectDataLoader::getObjectTypeFromName(objectIter.value()[0]);

                const ObjectData& objectData = ObjectDataLoader::getObjectData(roomObjectData.objectType);

                if (objectData.chestCapacity > 0)
                {
                    // Create inventory
                    loadChestContents(roomObjectData, objectIter);
                }

                roomData.objectsInRoom[objectIter.value()[1]] = roomObjectData;
            }
        }

        // roomNameToDataMap[iter.key()] = roomData;
        roomNameToTypeMap[iter.key()] = loaded_roomData.size();

        if (iter->contains("travel-location"))
        {
            roomData.isTravelLocation = true;
            roomTypeTravelLocationsMap[iter.key()] = loaded_roomData.size();
        }

        loaded_roomData.push_back(roomData);
    }

    int structureIdx = 0;

    // Load data
    auto structures = data.at("structures");
    for (nlohmann::ordered_json::iterator iter = structures.begin(); iter != structures.end(); ++iter)
    {
        StructureData structureData;
        auto jsonStructureData = iter.value();

        structureData.name = jsonStructureData.at("name");

        auto textureRect = jsonStructureData.at("texture");
        structureData.textureRect.left = textureRect[0];
        structureData.textureRect.top = textureRect[1];
        structureData.textureRect.width = textureRect[2];
        structureData.textureRect.height = textureRect[3];

        auto textureOrigin = jsonStructureData.at("texture-origin");
        structureData.textureOrigin.x = textureOrigin[0];
        structureData.textureOrigin.y = textureOrigin[1];

        structureData.size.x = jsonStructureData.at("size-x");
        structureData.size.y = jsonStructureData.at("size-y");

        structureData.collisionBitmaskOffset = jsonStructureData.at("collision-bitmask");
        structureData.lightBitmaskOffset = jsonStructureData.at("light-bitmask");

        structureData.roomType = roomNameToTypeMap[jsonStructureData.at("room")];

        loaded_structureData.push_back(structureData);

        structureNameToTypeMap[structureData.name] = structureIdx;

        structureIdx++;
    }

    return true;
}

void StructureDataLoader::loadChestContents(RoomObjectData& roomObjectData, nlohmann::ordered_json::iterator objectIter)
{
    auto chestInventoryContents = objectIter.value()[2];

    const ObjectData& objectData = ObjectDataLoader::getObjectData(roomObjectData.objectType);

    roomObjectData.chestContents = std::vector<InventoryData>();

    for (nlohmann::ordered_json::iterator chestInventory = chestInventoryContents.begin(); chestInventory != chestInventoryContents.end(); ++chestInventory)
    {
        InventoryData chestInventoryData(objectData.chestCapacity);

        for (auto itemIter = chestInventory.value().begin(); itemIter != chestInventory.value().end(); ++itemIter)
        {
            ItemType itemType = ItemDataLoader::getItemTypeFromName(itemIter.value()[1]);
            chestInventoryData.addItemAtIndex(itemIter.value()[0], itemType, itemIter.value()[2]);
        }

        roomObjectData.chestContents->push_back(chestInventoryData);
    }    
}

const StructureData& StructureDataLoader::getStructureData(StructureType type_index)
{    
    return loaded_structureData[type_index];
}

const RoomData& StructureDataLoader::getRoomData(RoomType roomType)
{
    return loaded_roomData[roomType];
}

ObjectType StructureDataLoader::getStructureTypeFromName(const std::string& structureName)
{
    return structureNameToTypeMap[structureName];
}

RoomType StructureDataLoader::getRoomTypeTravelLocationFromName(const std::string& roomLocationName)
{
    return roomTypeTravelLocationsMap[roomLocationName];
}