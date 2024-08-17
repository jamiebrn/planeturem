#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/Tween.hpp"
#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"
#include "Object/WorldObject.hpp"
#include "World/ChunkManager.hpp"
#include "Data/ToolData.hpp"
#include "Data/ToolDataLoader.hpp"

#include "GameConstants.hpp"

class Player : public WorldObject
{
public:
    Player(sf::Vector2f position);

    void update(float dt, sf::Vector2f mouseWorldPos, ChunkManager& chunkManager, int worldSize, bool& wrappedAroundWorld, sf::Vector2f& wrapPositionDelta);
    void draw(sf::RenderTarget& window, float dt, const sf::Color& color) override;
    void drawLightMask(sf::RenderTarget& lightTexture);

    void useTool();
    bool isUsingTool();

    bool canReachPosition(sf::Vector2f worldPos);

    void createPrompt(std::string text);

private:
    CollisionRect collisionRect;
    sf::Vector2f direction;
    bool flippedTexture;

    AnimatedTexture idleAnimation;
    AnimatedTexture runAnimation;

    static const sf::Vector2f toolOffset;

    int tileReach = 4;

    unsigned int equippedTool;

    // Tool animation
    float toolRotation;
    Tween<float> toolTweener;
    TweenID rotationTweenID;
    bool swingingTool;
    bool usingTool;

    // Prompts
    // String - prompt text
    // Float - prompt current lifetime
    std::vector<std::pair<std::string, float>> prompts;
    
};