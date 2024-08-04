#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

struct EntityData
{
    std::string name;
    int health;

    sf::IntRect textureRect;
    sf::Vector2f textureOrigin;

    // Size of entity IN TILES
    // e.g. (1, 1) size means screen size is (tileSize * scale, tileSize * scale)
    sf::Vector2f size;

    std::unordered_map<unsigned int, int> itemDrops;
};