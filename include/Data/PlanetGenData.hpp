#pragma once

#include <string>
#include <map>
#include <vector>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

#include "Data/typedefs.hpp"

struct TilemapData
{
    sf::Vector2i textureOffset;
    int variation;
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

    std::vector<TileGenData> tileGenDatas;
    //int tilemapStartID;

    std::vector<ObjectGenData> objectGenDatas;
    std::vector<EntityGenData> entityGenDatas;
    std::vector<StructureGenData> structureGenDatas;
    std::vector<FishCatchData> fishCatchDatas;

    float noiseRangeMin;
    float noiseRangeMax;
};

struct PlanetGenData
{
    std::string name;

    std::vector<BiomeGenData> biomeGenDatas;

    sf::Color waterColour;
};