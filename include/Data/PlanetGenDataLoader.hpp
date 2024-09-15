#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <fstream>
#include <string>
#include <unordered_map>

#include "Core/json.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/typedefs.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/EntityDataLoader.hpp"
#include "World/TileMap.hpp"

class PlanetGenDataLoader
{
    PlanetGenDataLoader() = delete;

public:
    static bool loadData(std::string planetGenDataPath);

    static const PlanetGenData& genPlanetGenData(PlanetType planetType);

    static PlanetType getPlanetTypeFromName(const std::string& planetName);

    static TileMap getTileMapFromID(int tileID);

private:
    static bool loadPlanet(nlohmann::ordered_json::iterator& planetData, const nlohmann::ordered_json& allPlanetGenData);

private:
    static std::vector<PlanetGenData> loaded_planetGenData;

    static std::unordered_map<std::string, PlanetType> planetStringToTypeMap;

    static std::unordered_map<int, TileMap> tileIdToTileMap;
};