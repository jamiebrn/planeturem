#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

struct ObjectData
{
    std::string name;
    int health;
    
    sf::IntRect textureRect;
    sf::Vector2f textureOrigin;

    bool hasCollision = false;
    int drawLayer = 0;

    std::unordered_map<unsigned int, int> itemDrops;
};