#pragma once

#include <SFML/Graphics.hpp>

#include "Core/SpriteBatch.hpp"

class Game;

class BossEntity
{
public:
    BossEntity() = default;

    virtual void update(Game& game, sf::Vector2f playerPos, float dt) = 0;

    virtual void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch) = 0;

    virtual void handleWorldWrap(sf::Vector2f positionDelta) = 0;
};