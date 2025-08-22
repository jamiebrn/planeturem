#pragma once

#include <vector>
#include <fstream>
#include <string>
#include <unordered_map>

#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/json.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/typedefs.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/EntityDataLoader.hpp"
#include "Data/StructureDataLoader.hpp"
#include "World/TileMap.hpp"

#include "GameConstants.hpp"
#include "DebugOptions.hpp"

#include "IO/Log.hpp"

class PlanetGenDataLoader
{
    PlanetGenDataLoader() = delete;

public:
    static bool loadData(std::string planetGenDataPath);

    static const PlanetGenData& getPlanetGenData(PlanetType planetType);

    static PlanetType getPlanetTypeFromName(const std::string& planetName);

    static const TileMapData& getTileMapDataFromID(int tileID);

    static const std::unordered_map<std::string, int>& getTileMapNameToIdMap();

    static inline const std::unordered_map<std::string, PlanetType>& getPlanetStringToTypeMap() {return planetStringToTypeMap;}

    static inline const std::string& getDataHash() {return dataHash;}

private:
    static bool loadPlanet(nlohmann::ordered_json::iterator& planetData, const nlohmann::ordered_json& allPlanetGenData);

private:
    static std::vector<PlanetGenData> loaded_planetGenData;

    static std::unordered_map<std::string, PlanetType> planetStringToTypeMap;

    static std::vector<TileMapData> tileMapDatas;
    static std::unordered_map<std::string, int> tileMapNameToId;

    static std::string dataHash;

};