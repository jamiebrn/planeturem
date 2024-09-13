#include "Data/PlanetGenDataLoader.hpp"

std::vector<PlanetGenData> PlanetGenDataLoader::loaded_planetGenData;

std::unordered_map<std::string, PlanetType> PlanetGenDataLoader::planetStringToTypeMap;

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

bool PlanetGenDataLoader::loadPlanet(nlohmann::ordered_json::iterator& planetData, const nlohmann::ordered_json& allPlanetGenData)
{
    PlanetGenData planetGenData;

    planetGenData.name = planetData.key();

    if (!planetData->contains("water-colour"))
        return false;
    
    planetGenData.waterColour.r = planetData->at("water-colour")[0];
    planetGenData.waterColour.g = planetData->at("water-colour")[1];
    planetGenData.waterColour.b = planetData->at("water-colour")[2];
    
    if (!planetData->contains("biomes"))
        return false;
    
    if (!allPlanetGenData.contains("tilemaps"))
        return false;
    
    // Load biomes
    auto biomes = planetData->at("biomes");
    auto tileMaps = allPlanetGenData.at("tilemaps");
    for (nlohmann::ordered_json::iterator biomeIter = biomes.begin(); biomeIter != biomes.end(); ++biomeIter)
    {
        BiomeGenData biomeGenData;

        if (!biomeIter->contains("name"))
            return false;
        
        biomeGenData.name = biomeIter->at("name");

        if (!biomeIter->contains("noise-range"))
            return false;
        
        // Load noise range
        auto noiseRange = biomeIter->at("noise-range");
        biomeGenData.noiseRangeMin = noiseRange[0];
        biomeGenData.noiseRangeMax = noiseRange[1];

        // Load biome tilemaps
        if (!biomeIter->contains("tiles"))
            return false;
        
        auto tiles = biomeIter->at("tiles");
        for (nlohmann::ordered_json::iterator tileIter = tiles.begin(); tileIter != tiles.end(); ++tileIter)
        {
            TileGenData tileGenData;

            // Get tilemap data
            if (!tileMaps.contains(tileIter.value()[0]))
                return false;
            
            if (tileIter->is_array())
            {
                const std::string& tileMapName = tileIter.value()[0];
                tileGenData.tileMap.textureOffset.x = tileMaps.at(tileMapName)[0];
                tileGenData.tileMap.textureOffset.y = tileMaps.at(tileMapName)[1];
                tileGenData.tileMap.variation = tileMaps.at(tileMapName)[2];
            }
            else
            {
                return false;
            }

            tileGenData.chanceRangeMin = tileIter.value()[1];
            tileGenData.chanceRangeMax = tileIter.value()[2];

            tileGenData.objectsCanSpawn = false;
            if (tileIter.value().size() > 3)
            {
               tileGenData.objectsCanSpawn = tileIter.value()[3];
            }

            biomeGenData.tileGenDatas.push_back(tileGenData);
        }

        // Load biome objects
        if (biomeIter->contains("objects"))
        {
            auto objects = biomeIter->at("objects");
            for (nlohmann::ordered_json::iterator objectIter = objects.begin(); objectIter != objects.end(); ++objectIter)
            {
                ObjectGenData objectGenData;

                objectGenData.object = ObjectDataLoader::getObjectTypeFromName(objectIter.value()[0]);
                objectGenData.spawnChance = objectIter.value()[1];

                biomeGenData.objectGenDatas.push_back(objectGenData);
            }
        }

        // Load biome entities
        if (biomeIter->contains("entities"))
        {
            auto entities = biomeIter->at("entities");
            for (nlohmann::ordered_json::iterator entityIter = entities.begin(); entityIter != entities.end(); ++entityIter)
            {
                EntityGenData entityGenData;

                entityGenData.entity = EntityDataLoader::getEntityTypeFromName(entityIter.value()[0]);

                entityGenData.spawnCountLow = entityIter.value()[1];
                entityGenData.spawnCountHigh = entityIter.value()[2];
                entityGenData.spawnChance = entityIter.value()[3];

                biomeGenData.entityGenDatas.push_back(entityGenData);
            }
        }

        planetGenData.biomeGenDatas.push_back(biomeGenData);
    }

    // Add to name-to-type map
    planetStringToTypeMap[planetGenData.name] = loaded_planetGenData.size();

    // Add planet data to array
    loaded_planetGenData.push_back(planetGenData);

    // Default case
    return true;
}

const PlanetGenData& PlanetGenDataLoader::genPlanetGenData(PlanetType planetType)
{
    return loaded_planetGenData[planetType];
}

PlanetType PlanetGenDataLoader::getPlanetTypeFromName(const std::string& planetName)
{
    return planetStringToTypeMap[planetName];
}