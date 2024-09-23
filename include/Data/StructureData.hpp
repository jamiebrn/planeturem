#pragma once

#include <SFML/Graphics.hpp>
#include <string>

struct RoomData
{
    sf::Vector2i tileSize;

    sf::IntRect textureRect;

    sf::Vector2i collisionBitmaskOffset;
};

struct StructureData
{
    std::string name;

    sf::Vector2i size;

    sf::IntRect textureRect;
    sf::Vector2f textureOrigin;

    sf::Vector2i collisionBitmaskOffset;

    RoomData roomData;
};