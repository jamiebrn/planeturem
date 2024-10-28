#pragma once

#include <utility>

typedef int ItemType;

// -10 Reserved for dummy object with collision
// -11 Reserved for dummy object with no collision
typedef int ObjectType;

typedef int EntityType;

typedef int ToolType;
typedef int ProjectileType;

typedef int ArmourType;

typedef std::pair<ItemType, unsigned int> ItemCount;

typedef int RoomType;
typedef int StructureType;
typedef int PlanetType;