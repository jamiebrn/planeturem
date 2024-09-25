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

        toolData.toolBehaviourType = getToolBehaviourTypeFromStr(jsonToolData.at("behaviour-type"));

        int textureWidth = jsonToolData.at("texture-width");
        int textureHeight = jsonToolData.at("texture-height");

        auto textures = jsonToolData.at("textures");
        for (nlohmann::json::iterator texturePositionIter = textures.begin(); texturePositionIter != textures.end(); ++texturePositionIter)
        {
            sf::IntRect textureRect;
            textureRect.left = texturePositionIter.value()[0];
            textureRect.top = texturePositionIter.value()[1];
            textureRect.width = textureWidth;
            textureRect.height = textureHeight;

            toolData.textureRects.push_back(textureRect);
        }

        if (jsonToolData.contains("pivot-x")) toolData.pivot.x = jsonToolData.at("pivot-x");
        if (jsonToolData.contains("pivot-y")) toolData.pivot.y = jsonToolData.at("pivot-y");

        if (jsonToolData.contains("damage")) toolData.damage = jsonToolData.at("damage");

        if (jsonToolData.contains("hold-offset"))
        {
            auto holdOffset = jsonToolData.at("hold-offset");
            toolData.holdOffset.x = holdOffset[0];
            toolData.holdOffset.y = holdOffset[1];
        }

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

ToolBehaviourType ToolDataLoader::getToolBehaviourTypeFromStr(const std::string& toolBehaviourStr)
{
    if (toolBehaviourStrTypeMap.contains(toolBehaviourStr))
    {
        return toolBehaviourStrTypeMap.at(toolBehaviourStr);
    }

    // Default case - tool behaviour type string not found
    return ToolBehaviourType::Pickaxe;
}