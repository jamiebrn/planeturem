#include "Data/StructureDataLoader.hpp"

std::vector<StructureData> StructureDataLoader::loaded_structureData;

bool StructureDataLoader::loadData(std::string structureDataPath)
{
    std::ifstream file(structureDataPath);
    nlohmann::ordered_json data = nlohmann::ordered_json::parse(file);

    int structureIdx = 0;

    // Load data
    for (nlohmann::ordered_json::iterator iter = data.begin(); iter != data.end(); ++iter)
    {
        StructureData structureData;
        auto jsonStructureData = iter.value();

        structureData.name = jsonStructureData.at("name");

        auto textureRect = jsonStructureData.at("texture");
        structureData.textureRect.left = textureRect[0];
        structureData.textureRect.top = textureRect[1];
        structureData.textureRect.width = textureRect[2];
        structureData.textureRect.height = textureRect[3];

        structureData.size.x = jsonStructureData.at("size-x");
        structureData.size.y = jsonStructureData.at("size-y");

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