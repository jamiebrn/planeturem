#pragma once

#include <SFML/Graphics.hpp>
#include <string>

typedef unsigned int ItemType;

struct ItemData
{
    std::string name;
    sf::IntRect textureRect;
};

struct ItemDrop
{
    ItemType item;
    unsigned int minAmount;
    unsigned int maxAmount;
    float chance;
};