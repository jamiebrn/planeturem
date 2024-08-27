#pragma once

#include <SFML/Graphics.hpp>
#include <string>

#include "Data/typedefs.hpp"
#include "Data/ObjectData.hpp"

struct ItemData
{
    std::string name;
    sf::IntRect textureRect;

    ObjectType placesObjectType = -1;
    ToolType toolType = -1;
    bool placesLand = false;
};