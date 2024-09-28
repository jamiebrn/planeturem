#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>
#include <unordered_map>

#include "Core/json.hpp"
#include "Data/StructureData.hpp"
#include "Data/typedefs.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"

#include "GameConstants.hpp"

class StructureDataLoader
{
    StructureDataLoader() = delete;

public:
    static bool loadData(std::string structureDataPath);

    static const StructureData& getStructureData(StructureType type_index);

    static StructureType getStructureTypeFromName(const std::string& structureName);

private:
    static std::vector<StructureData> loaded_structureData;

    static std::unordered_map<std::string, StructureType> structureNameToTypeMap;

};