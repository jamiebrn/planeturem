#pragma once

#include <SFML/Graphics.hpp>
#include <string>

#include "Data/typedefs.hpp"
#include "Data/ObjectData.hpp"

struct ItemData
{
    std::string name;
    std::string description;
    sf::IntRect textureRect;

    unsigned int maxStackSize = 99;

    ObjectType placesObjectType = -1;
    ToolType toolType = -1;
    ArmourType armourType = -1;
    ProjectileType projectileType = -1;
    bool placesLand = false;

    std::string summonsBoss;

    bool isMaterial = false;

    // 0, 0, 0 reserved for rainbow effect
    sf::Color nameColor = sf::Color(255, 255, 255);
};