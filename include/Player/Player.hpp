#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <array>

#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/Tween.hpp"
#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"
#include "Core/SpriteBatch.hpp"
#include "Object/WorldObject.hpp"
#include "World/ChunkManager.hpp"
#include "Data/typedefs.hpp"
#include "Data/ToolData.hpp"
#include "Data/ToolDataLoader.hpp"

#include "GameConstants.hpp"
#include "DebugOptions.hpp"

class Player : public WorldObject
{
public:
    Player(sf::Vector2f position);

    void update(float dt, sf::Vector2f mouseWorldPos, ChunkManager& chunkManager, int worldSize, bool& wrappedAroundWorld, sf::Vector2f& wrapPositionDelta);
    void updateInStructure(float dt, sf::Vector2f mouseWorldPos);

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) override;
    void drawLightMask(sf::RenderTarget& lightTexture);

    void setTool(ToolType toolType);
    ToolType getTool();

    void useTool();
    bool isUsingTool();

    bool canReachPosition(sf::Vector2f worldPos);

    void enterStructure();

    void setPosition(sf::Vector2f worldPos);

    const CollisionRect& getCollisionRect();

private:
    void updateDirection(sf::Vector2f mouseWorldPos);
    void updateAnimation(float dt);

private:
    CollisionRect collisionRect;
    sf::Vector2f direction;
    bool flippedTexture;

    AnimatedTexture idleAnimation;
    AnimatedTexture runAnimation;

    static const sf::Vector2f toolOffset;

    int tileReach = 4;
    float speed = 120.0f;

    ToolType equippedTool;

    // Tool animation
    float toolRotation;
    Tween<float> toolTweener;
    TweenID rotationTweenID;
    bool swingingTool;
    bool usingTool;

    static constexpr std::array<float, 5> runningShadowScale = {1.0f, 0.8f, 0.7f, 0.8f, 0.9f};
    
};