#pragma once

#include <iostream>
#include "WorldObject.hpp"

class Bush : public WorldObject
{
public:
    Bush(sf::Vector2f position) : WorldObject(position) {}

    inline void update(float dt) override {};
    void draw(sf::RenderWindow& window) override;

    inline void interact() override {std::cout << "interact with bush" << std::endl;}
    
    inline bool isAlive() override {return true;}

};