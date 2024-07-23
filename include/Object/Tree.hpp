#pragma once

#include <iostream>
#include "WorldObject.hpp"

class Tree : public WorldObject
{
public:
    Tree(sf::Vector2f position) : WorldObject(position) {}
    inline void update(float dt) override {};
    void draw(sf::RenderWindow& window) override;
    void interact() override;

private:
    float flash_amount = 0.0f;

};