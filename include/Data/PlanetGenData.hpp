#pragma once

#include <string>
#include <map>
#include <vector>
#include <unordered_set>
#include <map>

#include <Graphics/Color.hpp>
#include <Vector.hpp>

#include "Data/typedefs.hpp"

struct TilemapData
{
    pl::Vector2<int> textureOffset;
    int variation;
    pl::Color mapColor;
};

struct TileGenData
{
    TilemapData tileMap;
    float noiseRangeMin;
    float noiseRangeMax;
    bool objectsCanSpawn;
    int tileID;
};

struct ObjectGenData
{
    ObjectType object;
    float spawnChance;
};

struct EntityGenData
{
    EntityType entity;
    float spawnChance;
};

struct StructureGenData
{
    StructureType structure;
    float spawnChance;
};

struct FishCatchData
{
    ItemType itemCatch;
    int count;
    float chance;
};

struct BiomeGenData
{
    std::string name;

    std::map<uint16_t, TileGenData> tileGenDatas;

    std::vector<ObjectGenData> objectGenDatas;
    std::vector<EntityGenData> entityGenDatas;
    std::vector<StructureGenData> structureGenDatas;
    std::vector<FishCatchData> fishCatchDatas;

    pl::Color waterColor;

    float noiseRangeMin;
    float noiseRangeMax;

    float resourceRegenerationTimeMin;
    float resourceRegenerationTimeMax;
    float resourceRegenerationDensity;

    std::unordered_set<std::string> bossesSpawnAllowedNames;
};

struct PlanetGenData
{
    std::string name;
    std::string displayName;

    std::vector<BiomeGenData> biomeGenDatas;

    // sf::Color waterColor;
    pl::Vector2<int> waterTextureOffset;

    pl::Vector2<int> cliffTextureOffset;

    int worldSize;

    float heightNoiseFrequency = 0.1f;
    float biomeNoiseFrequency = 0.1f;
    float riverNoiseFrequency = 0.1f;

    float riverNoiseRangeMin = 0.0f;
    float riverNoiseRangeMax = 0.0f;

    std::unordered_set<std::string> bossesSpawnAllowedNames;
};