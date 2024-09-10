#pragma once

#include <vector>
#include <fstream>

#include "Core/json.hpp"
#include "Data/PlanetGenData.hpp"

class PlanetGenDataLoader
{
    PlanetGenDataLoader() = delete;

public:
    static bool loadData(std::string planetGenDataPath);

    static const PlanetGenData& genPlanetGenData(PlanetType planetType);

private:
    static bool loadPlanet(nlohmann::ordered_json::iterator planetData, const nlohmann::ordered_json& allPlanetGenData);

private:
    static std::vector<PlanetGenData> loaded_planetGenData;
};