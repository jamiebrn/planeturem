#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "Core/json.hpp"
#include "Data/Serialise/Vector2Serialise.hpp"
#include "Data/Serialise/ColorSerialise.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"

class ObjectDataLoader
{
    ObjectDataLoader() = delete;

public:
    static bool loadData(std::string objectDataPath);

    static bool loadRocketPlanetDestinations(const std::unordered_map<std::string, PlanetType>& planetStringToTypeMap,
        const std::unordered_map<std::string, RoomType>& roomStringToTypeMap);

    static const ObjectData& getObjectData(ObjectType objectType);

    static ObjectType getObjectTypeFromName(const std::string& objectName);

    inline static const std::unordered_map<std::string, ObjectType>& getObjectNameToTypeMap() {return objectNameToTypeMap;}

private:
    static std::vector<ObjectData> loaded_objectData;

    static std::unordered_map<std::string, ObjectType> objectNameToTypeMap;

};