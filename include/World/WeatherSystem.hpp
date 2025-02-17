#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <SFML/Graphics.hpp>

#include <vector>
#include <unordered_map>

#include "World/FastNoise.h"
#include "Core/AnimatedTexture.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/SpriteBatch.hpp"

#include "Object/WorldObject.hpp"

class Game;
class ChunkManager;

enum class WeatherType
{
    None,
    Rain
};

struct WeatherTypeData
{
    AnimatedTexture particleAnimatedTexture;
    float redLightBias, greenLightBias, blueLightBias;
    float fallAngle, fallSpeedMin, fallSpeedMax;
};

class WeatherParticle : public WorldObject
{    
public:
    WeatherParticle(sf::Vector2f position, const WeatherTypeData& weatherTypeData);

    // Returns false if should be destroyed
    bool update(float dt, const Camera& camera, ChunkManager& chunkManager);

    // Returns target position
    sf::Vector2f getPosition() const override;

    void handleWorldWrap(sf::Vector2f positionDelta);
    
    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
       const sf::Color& color) const override;
    
private:
    sf::Vector2f velocity;

    float fallTime;
    float targetFallTime;
    
    AnimatedTexture animatedTexture;

};
    
class WeatherSystem
{
public:
    WeatherSystem() = default;
    WeatherSystem(float gameTime, int seed);
    
    void update(float dt, float gameTime, const Camera& camera, ChunkManager& chunkManager);

    void handleWorldWrap(sf::Vector2f positionDelta);

    std::vector<WorldObject*> getWeatherParticles();

    const WeatherTypeData& getWeatherTypeData() const;

    float getRedLightBias() const;
    float getGreenLightBias() const;
    float getBlueLightBias() const;

    float sampleWeatherFunction(float gameTime) const;

    void presimulateWeather(float gameTime, const Camera& camera, ChunkManager& chunkManager);

    inline void setWeather(WeatherType type) {currentWeatherType = type;}

private:
    void updateCurrentWeather(float gameTime, bool initialise);
public:
    float getDestinationTransitionProgress() const;

public:
    static constexpr int PRESIMULATION_TICKS = 200;

private:
    static const std::unordered_map<WeatherType, WeatherTypeData> weatherTypeDatas;
    
    std::vector<WeatherParticle> weatherParticles;

    WeatherType currentWeatherType = WeatherType::None;
    WeatherType destinationWeatherType = WeatherType::None;

    FastNoise weatherNoise;

    static constexpr float LIGHT_BIAS_TRANSITION_SPEED = 0.05f;
    float redLightBias, greenLightBias, blueLightBias;

    static constexpr float PARTICLE_SPAWN_RATE = 0.005f;
    float particleSpawnTimer;

};