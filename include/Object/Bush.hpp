#pragma once

#include <iostream>
#include "WorldObject.hpp"
#include "Types/ObjectType.hpp"

class Bush : public WorldObject
{
public:
    Bush(sf::Vector2f position) : WorldObject(position) {}

    inline void update(float dt) override;
    void draw(sf::RenderWindow& window, float dt, const sf::Color& color) override;

    inline void interact() override;
    
    inline bool isAlive() override {return health > 0;}

    inline ObjectType getObjectType() override {return ObjectType::Bush;}

private:
    int health = 2;
    float flash_amount = 0.0f;

};