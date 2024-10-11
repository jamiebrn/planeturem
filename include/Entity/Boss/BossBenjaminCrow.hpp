#pragma once

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/SpriteBatch.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Camera.hpp"
#include "Core/Helper.hpp"
#include "Core/AnimatedTexture.hpp"

#include "BossEntity.hpp"

class BossBenjaminCrow : public BossEntity
{
public:
    BossBenjaminCrow(sf::Vector2f position);

    void update(Game& game, sf::Vector2f playerPos, float dt) override;

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch) override;

    void handleWorldWrap(sf::Vector2f positionDelta) override;

private:
    enum class BossBenjaminState
    {
        Idle,
        Dash
    };

private:
    sf::Vector2f position;
    sf::Vector2f direction;

    float flyingHeight;

    static constexpr float MOVE_SPEED = 50.0f;

    BossBenjaminState behaviourState;

    AnimatedTexture idleAnim;
};