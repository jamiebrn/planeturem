#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

#include "Core/Tween.hpp"

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/BuildableObjectPOD.hpp"
#include "Object/ParticleSystem.hpp"

#include "Data/typedefs.hpp"

class Game;

class RocketObject : public BuildableObject
{
public:
    RocketObject(sf::Vector2f position, ObjectType objectType);

    BuildableObject* clone() override;

    void update(Game& game, float dt, bool onWater, bool loopAnimation = true) override;

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const sf::Color& color) const override;

    void interact(Game& game) override;
    bool isInteractable() const override;

    void triggerBehaviour(Game& game, ObjectBehaviourTrigger trigger) override;

    void startFlyingUpwards();
    void startFlyingDownwards();

    void getRocketAvailableDestinations(PlanetType currentPlanetType, RoomType currentRoomType,
        std::vector<PlanetType>& planetDestinations, std::vector<RoomType>& roomDestinations);

    sf::Vector2f getRocketPosition();
    sf::Vector2f getRocketBottomPosition();

    void setRocketYOffset(float offset);
    float getRocketYOffset();

    void createRocketParticles();

private:
    void drawRocket(sf::RenderTarget& window, SpriteBatch& spriteBatch, const Camera& camera, const sf::Color& color) const;

private:
    bool flyingUp = false;
    bool flyingDown = false;
    bool entered = false;

    float rocketYOffset = 0.0f;

    Tween<float> floatTween;
    TweenID rocketFlyingTweenID;
    
    ParticleSystem particleSystem;
    float rocketParticleCooldown = 0.0f;
    static constexpr float ROCKET_PARTICLE_MAX_COOLDOWN = 0.02f;

};