#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

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

    sf::IntRect itemTexture;
    std::vector<sf::IntRect> wearTextures;
    sf::Vector2f wearTextureOffset = sf::Vector2f(0, 0);

    int defence = 0;
};
