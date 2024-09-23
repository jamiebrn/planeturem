#include "Data/StructureDataLoader.hpp"

std::vector<StructureData> StructureDataLoader::loaded_structureData;

std::unordered_map<std::string, StructureType> StructureDataLoader::structureNameToTypeMap;

bool StructureDataLoader::loadData(std::string structureDataPath)
{
    std::ifstream file(structureDataPath);
    nlohmann::ordered_json data = nlohmann::ordered_json::parse(file);

    // Load rooms
    std::unordered_map<std::string, RoomData> roomNameToDataMap;

    auto rooms = data.at("rooms");
    for (nlohmann::ordered_json::iterator iter = rooms.begin(); iter != rooms.end(); ++iter)
    {
        RoomData roomData;

        auto tileSize = iter->at("tile-size");
        roomData.tileSize.x = tileSize[0];
        roomData.tileSize.y = tileSize[1];

        auto textureOffset = iter->at("texture");
        roomData.textureRect.left = textureOffset[0];
        roomData.textureRect.top = textureOffset[1];
        roomData.textureRect.width = roomData.tileSize.x * TILE_SIZE_PIXELS_UNSCALED;
        roomData.textureRect.height = roomData.tileSize.y * TILE_SIZE_PIXELS_UNSCALED;

        auto collisionBitmask = iter->at("collision-bitmask");
        roomData.collisionBitmaskOffset.x = collisionBitmask[0];
        roomData.collisionBitmaskOffset.y = collisionBitmask[1];

        roomNameToDataMap[iter.key()] = roomData;
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

        auto collisionBitmask = jsonStructureData.at("collision-bitmask");
        structureData.collisionBitmaskOffset.x = collisionBitmask[0];
        structureData.collisionBitmaskOffset.y = collisionBitmask[1];

        structureData.roomData = roomNameToDataMap[jsonStructureData.at("room")];

        loaded_structureData.push_back(structureData);

        structureNameToTypeMap[structureData.name] = structureIdx;

        structureIdx++;
    }

    return true;
}

const StructureData& StructureDataLoader::getStructureData(StructureType type_index)
{    
    return loaded_structureData[type_index];
}

ObjectType StructureDataLoader::getStructureTypeFromName(const std::string& structureName)
{
    return structureNameToTypeMap[structureName];
}