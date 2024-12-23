#include "Data/ToolDataLoader.hpp"

std::vector<ToolData> ToolDataLoader::loaded_toolData;
std::unordered_map<std::string, ToolType> ToolDataLoader::toolNameToTypeMap;
std::vector<ProjectileData> ToolDataLoader::loaded_projectileData;
std::unordered_map<std::string, ProjectileType> ToolDataLoader::projectileNameToTypeMap;

bool ToolDataLoader::loadData(std::string toolDataPath)
{
    std::ifstream file(toolDataPath);
    nlohmann::json data = nlohmann::json::parse(file);

    // Load projectiles
    auto projectiles = data.at("projectiles");
    int projectileIdx = 0;

    for (nlohmann::json::iterator iter = projectiles.begin(); iter != projectiles.end(); ++iter)
    {
        ProjectileData projectileData;
        auto jsonProjectileData = iter.value();

        projectileData.name = jsonProjectileData.at("name");

        auto damageRange = jsonProjectileData.at("damage-range");
        projectileData.damageLow = damageRange[0];
        projectileData.damageHigh = damageRange[1];

        projectileData.speed = jsonProjectileData.at("speed");

        auto textureRectPos = jsonProjectileData.at("texture");

        projectileData.textureRect.top = textureRectPos[1];
        projectileData.textureRect.left = textureRectPos[0];
        projectileData.textureRect.width = jsonProjectileData.at("texture-width");
        projectileData.textureRect.height = jsonProjectileData.at("texture-height");

        auto origin = jsonProjectileData.at("origin");
        projectileData.origin.x = origin[0];
        projectileData.origin.y = origin[1];

        projectileNameToTypeMap[projectileData.name] = projectileIdx;

        float sellValue = 0;
        if (jsonProjectileData.contains("sell-value")) sellValue = jsonProjectileData.at("sell-value");

        if (jsonProjectileData.contains("collision-radius")) projectileData.collisionRadius = jsonProjectileData.at("collision-radius");
        if (jsonProjectileData.contains("collision-offset")) projectileData.collisionOffset = jsonProjectileData.at("collision-offset");

        // Link with item data
        projectileData.itemType = ItemDataLoader::createItemFromProjectile(projectileIdx, projectileData, sellValue);

        loaded_projectileData.push_back(projectileData);

        projectileIdx++;
    }


    // Load tool data
    auto tools = data.at("tools");
    int toolIdx = 0;
    for (nlohmann::json::iterator iter = tools.begin(); iter != tools.end(); ++iter)
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

        if (jsonToolData.contains("fishing-efficiency"))
        {
            toolData.fishingEfficiency = jsonToolData.at("fishing-efficiency");

            auto lineOffset = jsonToolData.at("fishing-rod-line-offset");
            toolData.fishingRodLineOffset.x = lineOffset[0];
            toolData.fishingRodLineOffset.y = lineOffset[1];
        }

        // Load projectile data if any
        if (jsonToolData.contains("projectiles"))
        {
            // Load weapon info
            toolData.projectileDamageMult = jsonToolData.at("projectile-damage-mult");
            toolData.shootPower = jsonToolData.at("shoot-power");

            // Load projectile types
            auto projectiles = jsonToolData.at("projectiles");
            for (auto iter = projectiles.begin(); iter != projectiles.end(); ++iter)
            {
                if (projectileNameToTypeMap.contains(iter.value()))
                {
                    toolData.projectileShootTypes.push_back(projectileNameToTypeMap[iter.value()]);
                }
            }
        }

        float sellValue = 0;
        if (jsonToolData.contains("sell-value")) sellValue = jsonToolData.at("sell-value");

        loaded_toolData.push_back(toolData);

        toolNameToTypeMap[toolData.name] = toolIdx;

        // Create item representing tool
        ItemDataLoader::createItemFromTool(toolData.name, toolIdx, sellValue);

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
    if (!toolNameToTypeMap.contains(toolName))
    {
        return 0;
    }
    
    return toolNameToTypeMap[toolName];
}

ProjectileType ToolDataLoader::getProjectileTypeFromName(const std::string& projectileName)
{
    if (!projectileNameToTypeMap.contains(projectileName))
    {
        return 0;
    }

    return projectileNameToTypeMap.at(projectileName);
}

const ProjectileData& ToolDataLoader::getProjectileData(ProjectileType projectile)
{
    return loaded_projectileData[projectile];
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