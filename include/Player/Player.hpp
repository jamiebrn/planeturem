#pragma once

#include <vector>
#include <math.h>
#include <algorithm>

#include "Object/WorldObject.hpp"
#include "Types/ObjectType.hpp"
#include "World/ChunkManager.hpp"
#include "Core/CollisionRect.hpp"

class Player : public WorldObject
{
public:
    Player(sf::Vector2f position);

    void update(float dt) override;
    void draw(sf::RenderWindow& window, float dt, const sf::Color& color) override;

    inline void interact() override {};

    // inline ObjectType getObjectType() override {return ObjectType::NONE;}
    
    inline bool isAlive() override {return true;}

private:
    CollisionRect collisionRect;
    
};