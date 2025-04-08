#pragma once

// #include <SFML/Graphics.hpp>
#include <fstream>
#include <string>
#include <unordered_map>

#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/json.hpp"
#include "Data/Serialise/Vector2Serialise.hpp"
#include "Data/typedefs.hpp"
#include "Data/ToolData.hpp"
#include "Data/ItemDataLoader.hpp"

class ToolDataLoader
{
    ToolDataLoader() = delete;

public:
    static bool loadData(std::string toolDataPath);

    static const ToolData& getToolData(ToolType tool);

    static ToolType getToolTypeFromName(const std::string& toolName);

    static ProjectileType getProjectileTypeFromName(const std::string& projectileName);

    static const ProjectileData& getProjectileData(ProjectileType projectile);

private:
    static ToolBehaviourType getToolBehaviourTypeFromStr(const std::string& toolBehaviourStr);

private:
    static std::vector<ToolData> loaded_toolData;

    static std::unordered_map<std::string, ToolType> toolNameToTypeMap;

    static std::vector<ProjectileData> loaded_projectileData;

    static std::unordered_map<std::string, ProjectileType> projectileNameToTypeMap;

};