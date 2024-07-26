#pragma once

#include <SFML/Graphics.hpp>
#include <string>

struct ObjectData
{
    std::string name;
    int health;
    
    sf::IntRect textureRect;
    sf::Vector2f textureOrigin;

    bool hasCollision = false;
    int drawLayer = 0;
};