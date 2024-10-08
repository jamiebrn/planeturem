#pragma once

#include <SFML/Graphics.hpp>

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"

class RocketObject : public BuildableObject
{
public:
    RocketObject(sf::Vector2f position, ObjectType objectType);

    BuildableObject* clone() override;

    void update(float dt, bool onWater, bool loopAnimation = true) override;

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) const override;

    ObjectInteractionType interact() const override;

    sf::Vector2f getRocketPosition();
    sf::Vector2f getRocketBottomPosition();

    void setRocketYOffset(float offset);
    float getRocketYOffset();

    void createRocketParticles(ParticleSystem& particleSystem);

private:
    void drawRocket(sf::RenderTarget& window, SpriteBatch& spriteBatch, const sf::Color& color) const;

private:
    float rocketYOffset = 0.0f;

};