#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

#include "Data/ItemDataLoader.hpp"

struct ObjectData
{
    std::string name;
    int health;
    
    sf::IntRect textureRect;
    sf::Vector2f textureOrigin;

    sf::Vector2i size = {1, 1};

    bool hasCollision = false;
    bool placeOnWater = false;

    int drawLayer = 0;

    float furnaceSpeed = -1;

    std::unordered_map<ItemType, unsigned int> itemDrops;
};