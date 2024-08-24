#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "Core/json.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"

class ObjectDataLoader
{
    ObjectDataLoader() = delete;

public:
    static bool loadData(std::string objectDataPath);

    static const ObjectData& getObjectData(ObjectType type_index);

    static ObjectType getObjectTypeFromName(const std::string& objectName);

private:
    static std::vector<ObjectData> loaded_objectData;

    static std::unordered_map<std::string, ObjectType> objectNameToTypeMap;

};