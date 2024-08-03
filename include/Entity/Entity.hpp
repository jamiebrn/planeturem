#pragma once

#include <SFML/Graphics.hpp>

#include "Object/WorldObject.hpp"

class Entity : public WorldObject
{
public:
    Entity(sf::Vector2f position) : WorldObject(position) {}

};