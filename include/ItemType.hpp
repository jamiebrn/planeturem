#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

#include "TextureType.hpp"

enum ItemType
{
    Wood
};

struct ItemTypeData
{
    std::string name;
    sf::IntRect textureRect;
};

inline const std::unordered_map<ItemType, ItemTypeData> ItemDataMap = {
    {ItemType::Wood, {"Wood", sf::IntRect(0, 0, 16, 16)}}
};