#pragma once

#include <SFML/Graphics.hpp>
#include <string>

struct StructureData
{
    std::string name;

    sf::Vector2i size;

    sf::IntRect textureRect;

    sf::Vector2i collisionBitmaskOffset;
};