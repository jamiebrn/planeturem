#pragma once

#include <string>
#include <map>

#include <SFML/System/Vector2.hpp>

#include "Data/typedefs.hpp"

struct TilemapData
{
    sf::Vector2i textureOffset;
    int variation;
};

struct TileGenerationData
{
    TilemapData tileMap;
    float chanceRangeMin;
    float chanceRangeMax;
    bool objectsCanSpawn;
};

struct ObjectGenerationData
{
    ObjectType object;
    float spawnChance;
};

struct EntityGenerationData
{
    EntityType entity;
    int spawnCountLow;
    int spawnCountHigh;
    float spawnChance;
};

struct BiomeGenerationData
{
    std::string name;

    // std::map
};

struct PlanetGenerationData
{

};