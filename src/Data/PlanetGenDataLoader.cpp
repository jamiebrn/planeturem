#include "Data/PlanetGenDataLoader.hpp"
#include <extlib/hashpp.h>

std::vector<PlanetGenData> PlanetGenDataLoader::loaded_planetGenData;

std::unordered_map<std::string, PlanetType> PlanetGenDataLoader::planetStringToTypeMap;

std::vector<TileMapData> PlanetGenDataLoader::tileMapDatas;
std::unordered_map<std::string, int> PlanetGenDataLoader::tileMapNameToId;

std::string PlanetGenDataLoader::dataHash;

bool PlanetGenDataLoader::loadData(std::string planetGenDataPath)
{
    std::ifstream file(planetGenDataPath);
    nlohmann::ordered_json data = nlohmann::ordered_json::parse(file);

    if (!data.contains("tilemaps"))
        return false;

    // Load tilemaps
    auto tileMaps = data.at("tilemaps");
    for (nlohmann::ordered_json::iterator tileMapIter = tileMaps.begin(); tileMapIter != tileMaps.end(); ++tileMapIter)
    {
        tileMapNameToId[tileMapIter.key()] = tileMapDatas.size() + 1;

        TileMapData tileMapData;
        tileMapData.name = tileMapIter.key();
        tileMapData.tileID = tileMapDatas.size() + 1;
        tileMapData.textureOffset.x = tileMapIter.value()[0];
        tileMapData.textureOffset.y = tileMapIter.value()[1];
        tileMapData.variation = tileMapIter.value()[2];
        tileMapData.mapColor = tileMapIter.value()[3];
        tileMapData.drawLayer = tileMapIter.value()[4];

        #if (!RELEASE_BUILD)
        DebugOptions::tileMapsVisible[tileMapData.tileID] = true;
        printf("Loaded tile id %d, %s\n", tileMapData.tileID, tileMapIter.key().c_str());
        #endif

        tileMapDatas.push_back(tileMapData);
    }

    if (!data.contains("planets"))
        return false;
    
    auto planets = data.at("planets");
    for (nlohmann::ordered_json::iterator iter = planets.begin(); iter != planets.end(); ++iter)
    {
        if (!loadPlanet(iter, data))
            return false;
    }

    dataHash = hashpp::get::getFileHash(hashpp::ALGORITHMS::MD5, planetGenDataPath);

    return true;
}

bool PlanetGenDataLoader::loadPlanet(nlohmann::ordered_json::iterator& planetData, const nlohmann::ordered_json& allPlanetGenData)
{
    PlanetGenData planetGenData;

    planetGenData.name = planetData.key();

    if (planetData->contains("display-name"))
    {
        planetGenData.displayName = planetData->at("display-name");
    }
    else
    {
        planetGenData.displayName = planetGenData.name;
    }

    planetGenData.waterTextureOffset = planetData->at("water-texture-offset");
    planetGenData.cliffTextureOffset = planetData->at("cliff-texture-offset");

    if (planetData->contains("height-noise-frequency")) planetGenData.heightNoiseFrequency = planetData->at("height-noise-frequency");
    if (planetData->contains("biome-noise-frequency")) planetGenData.biomeNoiseFrequency = planetData->at("biome-noise-frequency");
    if (planetData->contains("river-noise-frequency")) planetGenData.riverNoiseFrequency = planetData->at("river-noise-frequency");

    if (planetData->contains("river-noise-range"))
    {
        planetGenData.riverNoiseRangeMin = planetData->at("river-noise-range")[0];
        planetGenData.riverNoiseRangeMax = planetData->at("river-noise-range")[1];
    }

    if (!planetData->contains("size"))
        return false;
    
    planetGenData.worldSize = planetData->at("size");

    if (planetData->contains("bosses-spawn-allowed"))
    {
        planetGenData.bossesSpawnAllowedNames = planetData->at("bosses-spawn-allowed");
    }

    if (!planetData->contains("biomes"))
        return false;
    
    if (!allPlanetGenData.contains("tilemaps"))
        return false;
    
    if (planetData->contains("achievement-unlock-on-travel"))
    {
        planetGenData.achievementUnlockOnTravel = planetData->at("achievement-unlock-on-travel");
    }

    // Load biomes
    auto biomes = planetData->at("biomes");
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

        if (biomeIter->contains("water-colour"))
        {
            biomeGenData.waterColor.r = biomeIter->at("water-colour")[0];
            biomeGenData.waterColor.g = biomeIter->at("water-colour")[1];
            biomeGenData.waterColor.b = biomeIter->at("water-colour")[2];
        }
        else
        {
            biomeGenData.waterColor = pl::Color(255, 255, 255);
        }

        auto resourceRegenTime = biomeIter->at("resource-regeneration-time");
        biomeGenData.resourceRegenerationTimeMin = resourceRegenTime[0];
        biomeGenData.resourceRegenerationTimeMax = resourceRegenTime[1];

        biomeGenData.resourceRegenerationDensity = biomeIter->at("resource-regeneration-density");

        // Load biome tilemaps
        if (!biomeIter->contains("tiles"))
            return false;
        
        auto tiles = biomeIter->at("tiles");
        for (nlohmann::ordered_json::iterator tileIter = tiles.begin(); tileIter != tiles.end(); ++tileIter)
        {
            TileGenData tileGenData;

            // Get tilemap data
            if (!tileMapNameToId.contains(tileIter.value()[0]))
                return false;
            
            tileGenData.tileID = tileMapNameToId.at(tileIter.value()[0]);

            tileGenData.noiseRangeMin = tileIter.value()[1];
            tileGenData.noiseRangeMax = tileIter.value()[2];

            tileGenData.objectsCanSpawn = true;
            if (tileIter.value().size() > 3)
            {
               tileGenData.objectsCanSpawn = tileIter.value()[3];
            }

            biomeGenData.tileGenDatas[tileGenData.tileID] = tileGenData;
            biomeGenData.tileGenDataDrawOrder.push_back(tileGenData.tileID);
        }

        // Sort tile gen datas in draw order
        std::sort(biomeGenData.tileGenDataDrawOrder.begin(), biomeGenData.tileGenDataDrawOrder.end(), [](int tileMapIdA, int tileMapIdB)
        {
            int drawLayerA = PlanetGenDataLoader::getTileMapDataFromID(tileMapIdA).drawLayer;
            int drawLayerB = PlanetGenDataLoader::getTileMapDataFromID(tileMapIdB).drawLayer;
            if (drawLayerA == drawLayerB) return tileMapIdA < tileMapIdB;
            return drawLayerA < drawLayerB;
        });

        printf("Biome %s\n", biomeGenData.name.c_str());
        for (int tileMapID : biomeGenData.tileGenDataDrawOrder)
        {
            printf(" - Tile map \"%s\"\n", PlanetGenDataLoader::getTileMapDataFromID(tileMapID).name.c_str());
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
            // Get total fish chance then normalise after loading
            float totalChance = 0.0f;

            auto fishCatches = biomeIter->at("fish-catches");
            for (nlohmann::ordered_json::iterator fishCatchIter = fishCatches.begin(); fishCatchIter != fishCatches.end(); ++fishCatchIter)
            {
                FishCatchData fishCatchData;

                fishCatchData.itemCatch = ItemDataLoader::getItemTypeFromName(fishCatchIter.value()[0]);
                fishCatchData.count = fishCatchIter.value()[1];
                fishCatchData.chance = fishCatchIter.value()[2];

                totalChance += fishCatchData.chance;

                biomeGenData.fishCatchDatas.push_back(fishCatchData);
            }

            // Normalise probabilities
            for (FishCatchData& fishCatchData : biomeGenData.fishCatchDatas)
            {
                fishCatchData.chance /= totalChance;
            }
        }

        if (biomeIter->contains("bosses-spawn-allowed"))
        {
            biomeGenData.bossesSpawnAllowedNames = biomeIter->at("bosses-spawn-allowed");
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

const PlanetGenData& PlanetGenDataLoader::getPlanetGenData(PlanetType planetType)
{
    return loaded_planetGenData[planetType];
}

PlanetType PlanetGenDataLoader::getPlanetTypeFromName(const std::string& planetName)
{
    return planetStringToTypeMap[planetName];
}

const TileMapData& PlanetGenDataLoader::getTileMapDataFromID(int tileID)
{
    return tileMapDatas.at(tileID - 1);
}

const std::unordered_map<std::string, int>& PlanetGenDataLoader::getTileMapNameToIdMap()
{
    return tileMapNameToId;
}