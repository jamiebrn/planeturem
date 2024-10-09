#pragma once

#include <SFML/Graphics.hpp>

#include "Core/Tween.hpp"

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/BuildableObjectPOD.hpp"
#include "Object/ParticleSystem.hpp"

class RocketObject : public BuildableObject
{
public:
    RocketObject(sf::Vector2f position, ObjectType objectType);

    BuildableObject* clone() override;

    void update(float dt, bool onWater, bool loopAnimation = true) override;

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) const override;

    ObjectInteractionType interact() const override;

    void startFlyingUpwards();
    void startFlyingDownwards();
    bool finishedFlying();

    sf::Vector2f getRocketPosition();
    sf::Vector2f getRocketBottomPosition();

    void setRocketYOffset(float offset);
    float getRocketYOffset();

    void createRocketParticles();

private:
    void drawRocket(sf::RenderTarget& window, SpriteBatch& spriteBatch, const sf::Color& color) const;

private:
    bool flying = false;

    float rocketYOffset = 0.0f;

    Tween<float> floatTween;
    TweenID rocketFlyingTweenID;
    
    ParticleSystem particleSystem;
    float rocketParticleCooldown = 0.0f;
    static constexpr float ROCKET_PARTICLE_MAX_COOLDOWN = 0.02f;

};