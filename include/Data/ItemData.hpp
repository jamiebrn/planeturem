#pragma once

#include <SFML/Graphics.hpp>
#include <string>

typedef unsigned int ItemType;

struct ItemData
{
    std::string name;
    sf::IntRect textureRect;
};