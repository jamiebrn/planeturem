#pragma once

#include <map>

#include "ObjectType.hpp"

#include "ItemType.hpp"

inline const std::map<ObjectType, std::map<ItemType, int>> BuildRecipes = {
    {ObjectType::WoodWall, {{ItemType::Wood, 4}}}
};
