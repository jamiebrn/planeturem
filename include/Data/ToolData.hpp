#pragma once

#include <SFML/Graphics.hpp>

struct ToolData
{
    std::string name;

    sf::IntRect textureRect;
    sf::Vector2f pivot;

    int damage = 1;
};