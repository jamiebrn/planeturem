#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>
#include <iostream>

#include "Core/json.hpp"
#include "Data/ObjectData.hpp"

class ObjectDataLoader
{
    ObjectDataLoader() = delete;

public:
    static bool loadData(std::string objectDataPath);

    static const ObjectData& getObjectData(ObjectType type_index);

private:
    static std::vector<ObjectData> loaded_objectData;

};