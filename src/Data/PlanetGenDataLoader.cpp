#include "Data/PlanetGenDataLoader.hpp"

std::vector<PlanetGenData> PlanetGenDataLoader::loaded_planetGenData;

std::unordered_map<std::string, PlanetType> PlanetGenDataLoader::planetStringToTypeMap;

std::unordered_map<int, TileMap> PlanetGenDataLoader::tileIdToTileMap;

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
            
            tileGenData.tileID = tileIter.value()[1].get<int>() + 1;

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

            tileGenData.noiseRangeMin = tileIter.value()[2];
            tileGenData.noiseRangeMax = tileIter.value()[3];

            tileGenData.objectsCanSpawn = true;
            if (tileIter.value().size() > 4)
            {
               tileGenData.objectsCanSpawn = tileIter.value()[4];
            }

            // Store copy of blank tilemap against tileID in map
            if (tileIdToTileMap.count(tileGenData.tileID) == 0)
            {
                tileIdToTileMap[tileGenData.tileID] = TileMap(tileGenData.tileMap.textureOffset, tileGenData.tileMap.variation);
                DebugOptions::tileMapsVisible[tileGenData.tileID] = true;
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
                entityGenData.spawnChance = entityIter.value()[1];

                biomeGenData.entityGenDatas.push_back(entityGenData);
            }
        }

        // Load biome structures
        if (biomeIter->contains("structures"))
        {
            auto structures = biomeIter->at("structures");
            for (nlohmann::ordered_json::iterator structureIter = structures.begin(); structureIter != structures.end(); ++structureIter)
            {
                StructureGenData structureGenData;

                structureGenData.structure = StructureDataLoader::getStructureTypeFromName(structureIter.value()[0]);
                structureGenData.spawnChance = structureIter.value()[1];

                biomeGenData.structureGenDatas.push_back(structureGenData);
            }
        }

        if (biomeIter->contains("fish-catches"))
        {
            auto fishCatches = biomeIter->at("fish-catches");
            for (nlohmann::ordered_json::iterator fishCatchIter = fishCatches.begin(); fishCatchIter != fishCatches.end(); ++fishCatchIter)
            {
                FishCatchData fishCatchData;

                fishCatchData.itemCatch = ItemDataLoader::getItemTypeFromName(fishCatchIter.value()[0]);
                fishCatchData.count = fishCatchIter.value()[1];
                fishCatchData.chance = fishCatchIter.value()[2];

                biomeGenData.fishCatchDatas.push_back(fishCatchData);
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

TileMap PlanetGenDataLoader::getTileMapFromID(int tileID)
{
    return tileIdToTileMap[tileID];
}