#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>
#include <unordered_map>

#include "Core/json.hpp"
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

private:
    static std::vector<EntityData> loaded_entityData;

    static std::unordered_map<std::string, EntityType> entityNameToTypeMap;

};