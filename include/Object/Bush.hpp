#pragma once

#include <iostream>
#include "WorldObject.hpp"

class Bush : public WorldObject
{
public:
    Bush(sf::Vector2f position) : WorldObject(position) {}

    inline void update(float dt) override;
    void draw(sf::RenderWindow& window, float dt, float alpha) override;

    inline void interact() override;
    
    inline bool isAlive() override {return health > 0;}

private:
    int health = 2;
    float flash_amount = 0.0f;

};