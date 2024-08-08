#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/Tween.hpp"
#include "Object/WorldObject.hpp"
#include "World/ChunkManager.hpp"
#include "Data/ToolData.hpp"
#include "Data/ToolDataLoader.hpp"

class Player : public WorldObject
{
public:
    Player(sf::Vector2f position);

    void update(float dt, sf::Vector2f mouseWorldPos, ChunkManager& chunkManager);
    void draw(sf::RenderWindow& window, float dt, const sf::Color& color) override;

    void useTool();
    bool isUsingTool();

private:
    CollisionRect collisionRect;
    sf::Vector2f direction;
    bool flippedTexture;

    AnimatedTexture idleAnimation;
    AnimatedTexture runAnimation;

    static const sf::Vector2f toolOffset;

    unsigned int equippedTool;
    float toolRotation;
    Tween<float> toolTweener;
    TweenID rotationTweenID;
    bool swingingTool;
    bool usingTool;
    
};