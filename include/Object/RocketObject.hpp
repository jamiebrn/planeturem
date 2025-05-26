#pragma once

#include <vector>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Tween.hpp"

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/BuildableObjectPOD.hpp"
#include "Object/ParticleSystem.hpp"
#include "Player/LocationState.hpp"

#include "Data/typedefs.hpp"

class Game;
class NetworkHandler;

class RocketObject : public BuildableObject
{
public:
    RocketObject(pl::Vector2f position, ObjectType objectType, const BuildableObjectCreateParameters& parameters);

    BuildableObject* clone() override;

    void update(Game& game, float dt, bool onWater, bool loopAnimation = true) override;

    bool damage(int amount, Game& game, ChunkManager& chunkManager, ParticleSystem* particleSystem, bool giveItems = true, bool createHitMarkers = true) override;

    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const pl::Color& color) const override;

    void interact(Game& game, bool isClient) override;
    bool isInteractable() const override;

    void startFlyingUpwards(Game& game, const LocationState& locationState, NetworkHandler* networkHandler);
    void startFlyingDownwards(Game& game, const LocationState& locationState, NetworkHandler* networkHandler, bool enterRocket);

    void enter();
    void exit();
    bool isEntered();

    void getRocketAvailableDestinations(PlanetType currentPlanetType, RoomType currentRoomType,
        std::vector<PlanetType>& planetDestinations, std::vector<RoomType>& roomDestinations);

    pl::Vector2f getRocketPosition();
    pl::Vector2f getRocketBottomPosition();

    void setRocketYOffset(float offset);
    float getRocketYOffset();

    void createRocketParticles();

private:
    void drawRocket(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, int worldSize, const pl::Color& color) const;

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