#pragma once

#include "WorldObject.hpp"

class Bush : public WorldObject
{
public:
    Bush(sf::Vector2f position) : position(position) {}
    void update(float dt) {};
    void draw(sf::RenderWindow& window);

private:
    sf::Vector2f position;

};