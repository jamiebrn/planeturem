#pragma once

#include "WorldObject.hpp"
#include "math.h"

class Player : public WorldObject
{
public:
    Player(sf::Vector2f position) : WorldObject(position) {}
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    inline void interact() override {};
    
};