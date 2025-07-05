#pragma once

#include <fstream>
#include <vector>
#include <unordered_map>
#include <iostream>

#include <Vector.hpp>
#include <Rect.hpp>

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

    static inline const std::unordered_map<std::string, ObjectType>& getObjectNameToTypeMap() {return objectNameToTypeMap;}
    
    static inline const std::string& getDataHash() {return dataHash;}

private:
    static std::vector<ObjectData> loaded_objectData;

    static std::unordered_map<std::string, ObjectType> objectNameToTypeMap;

    static std::string dataHash;

};