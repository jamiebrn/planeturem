#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>

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

private:
    static std::vector<EntityData> loaded_entityData;

};