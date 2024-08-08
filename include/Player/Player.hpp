#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Object/WorldObject.hpp"
#include "World/ChunkManager.hpp"
#include "Data/ToolData.hpp"
#include "Data/ToolDataLoader.hpp"

class Player : public WorldObject
{
public:
    Player(sf::Vector2f position);

    void update(float dt, ChunkManager& chunkManager);
    void draw(sf::RenderWindow& window, float dt, const sf::Color& color) override;

private:
    CollisionRect collisionRect;
    sf::Vector2f direction;
    bool flippedTexture;

    AnimatedTexture idleAnimation;
    AnimatedTexture runAnimation;

    unsigned int equippedTool;
    float t = 0;
    
};