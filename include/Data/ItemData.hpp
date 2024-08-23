#pragma once

#include <SFML/Graphics.hpp>
#include <string>

typedef int ItemType;

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