#include "Data/PlanetGenDataLoader.hpp"

bool PlanetGenDataLoader::loadData(std::string planetGenDataPath)
{
    std::ifstream file(planetGenDataPath);
    nlohmann::ordered_json data = nlohmann::ordered_json::parse(file);

    if (!data.contains("planets"))
        return false;
    
    auto planets = data.at("planets");
    for (nlohmann::ordered_json::iterator iter = planets.begin(); iter != planets.end(); ++iter)
    {
        if (!loadPlanet(iter, data))
            return false;
    }

    return true;   
}

bool PlanetGenDataLoader::loadPlanet(nlohmann::ordered_json::iterator planetData, const nlohmann::ordered_json& allPlanetGenData)
{
    PlanetGenData planetGenData;
    
    if (!allPlanetGenData.contains("biomes"))
        return false;
    
    
}

const PlanetGenData& PlanetGenDataLoader::genPlanetGenData(PlanetType planetType)
{
    return loaded_planetGenData[planetType];
}