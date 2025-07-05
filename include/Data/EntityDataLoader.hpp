#pragma once

#include <fstream>
#include <vector>
#include <unordered_map>

#include "Core/json.hpp"
#include "Data/Serialise/Vector2Serialise.hpp"
#include "Data/Serialise/IntRectSerialise.hpp"
#include "Data/EntityData.hpp"
#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ItemDrop.hpp"

class EntityDataLoader
{
    EntityDataLoader() = delete;

public:
    static bool loadData(std::string entityDataPath);

    static const EntityData& getEntityData(EntityType entity);

    static EntityType getEntityTypeFromName(const std::string& entityName);
    
    static inline const std::string& getDataHash() {return dataHash;}

private:
    static std::vector<EntityData> loaded_entityData;

    static std::unordered_map<std::string, EntityType> entityNameToTypeMap;

    static std::string dataHash;

};