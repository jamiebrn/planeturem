#include "Data/ToolDataLoader.hpp"

std::vector<ToolData> ToolDataLoader::loaded_toolData;
std::unordered_map<std::string, ToolType> ToolDataLoader::toolNameToTypeMap;

bool ToolDataLoader::loadData(std::string toolDataPath)
{
    std::ifstream file(toolDataPath);
    nlohmann::json data = nlohmann::json::parse(file);

    int toolIdx = 0;

    // Load data
    for (nlohmann::json::iterator iter = data.begin(); iter != data.end(); ++iter)
    {
        ToolData toolData;
        auto jsonToolData = iter.value();

        toolData.name = jsonToolData.at("name");

        if (jsonToolData.contains("texture-x")) toolData.textureRect.left = jsonToolData.at("texture-x");
        if (jsonToolData.contains("texture-y")) toolData.textureRect.top = jsonToolData.at("texture-y");
        if (jsonToolData.contains("texture-width")) toolData.textureRect.width = jsonToolData.at("texture-width");
        if (jsonToolData.contains("texture-height")) toolData.textureRect.height = jsonToolData.at("texture-height");

        if (jsonToolData.contains("pivot-x")) toolData.pivot.x = jsonToolData.at("pivot-x");
        if (jsonToolData.contains("pivot-y")) toolData.pivot.y = jsonToolData.at("pivot-y");

        if (jsonToolData.contains("damage")) toolData.damage = jsonToolData.at("damage");

        loaded_toolData.push_back(toolData);

        toolNameToTypeMap[toolData.name] = toolIdx;

        // Create item representing tool
        ItemDataLoader::createItemFromTool(toolData.name, toolIdx);

        toolIdx++;
    }

    return true;
}

const ToolData& ToolDataLoader::getToolData(ToolType tool)
{
    return loaded_toolData[tool];
}

ToolType ToolDataLoader::getToolTypeFromName(const std::string& toolName)
{
    return toolNameToTypeMap[toolName];
}