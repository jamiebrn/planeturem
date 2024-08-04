#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>

#include "Core/json.hpp"
#include "Data/EntityData.hpp"

class EntityDataLoader
{
    EntityDataLoader() = delete;

public:
    static bool loadData(std::string entityDataPath);

    static const EntityData& getEntityData(unsigned int type_index);

private:
    static std::vector<EntityData> loaded_entityData;

};