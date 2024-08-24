#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"

struct EntityData
{
    std::string name;
    int health;

    // sf::IntRect textureRect;
    std::vector<sf::IntRect> idleTextureRects;
    std::vector<sf::IntRect> walkTextureRects;
    float idleAnimSpeed;
    float walkAnimSpeed;

    sf::Vector2f textureOrigin;

    // Size of entity IN TILES
    // e.g. (1, 1) size means screen size is (tileSize * scale, tileSize * scale)
    sf::Vector2f size;

    std::unordered_map<ItemType, unsigned int> itemDrops;
};