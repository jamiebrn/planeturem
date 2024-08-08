#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <string>

#include "Core/json.hpp"
#include "Data/ToolData.hpp"

class ToolDataLoader
{
    ToolDataLoader() = delete;

public:
    static bool loadData(std::string toolDataPath);

    static const ToolData& getToolData(int index);

private:
    static std::vector<ToolData> loaded_toolData;

};