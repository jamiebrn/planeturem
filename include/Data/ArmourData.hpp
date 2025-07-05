#pragma once

#include <string>
#include <vector>

#include <Vector.hpp>
#include <Rect.hpp>

#include "Data/typedefs.hpp"

enum class ArmourWearType
{
    Head,
    Chest,
    Feet
};

struct ArmourData
{
    std::string name;
    ArmourWearType armourWearType;

    pl::Rect<int> itemTexture;
    std::vector<pl::Rect<int>> wearTextures;
    pl::Vector2f wearTextureOffset = pl::Vector2f(0, 0);

    int defence = 0;
};
