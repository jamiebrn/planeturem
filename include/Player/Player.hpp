#pragma once

#include <vector>
#include <math.h>
#include <algorithm>

#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Object/WorldObject.hpp"
#include "World/ChunkManager.hpp"

class Player : public WorldObject
{
public:
    Player(sf::Vector2f position);

    void update(float dt, ChunkManager& chunkManager);
    void draw(sf::RenderWindow& window, float dt, const sf::Color& color) override;

    inline void interact() override {};
    
    inline bool isAlive() override {return true;}

private:
    CollisionRect collisionRect;
    
};