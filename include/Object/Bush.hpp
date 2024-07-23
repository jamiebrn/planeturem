#pragma once

#include "WorldObject.hpp"

class Bush : public WorldObject
{
public:
    Bush(sf::Vector2f position) : WorldObject(position) {}
    void update(float dt) {};
    void draw(sf::RenderWindow& window);

};