#pragma once

#include <SFML/Graphics.hpp>

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <unordered_map>

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
    WeatherSystem();
    
    void update(float dt, const Camera& camera, ChunkManager& chunkManager);

    void handleWorldWrap(sf::Vector2f positionDelta);

    std::vector<WorldObject*> getWeatherParticles();

    const WeatherTypeData& getWeatherTypeData() const;

    inline void setWeather(WeatherType type) {currentWeatherType = type;}

private:
    static const std::unordered_map<WeatherType, WeatherTypeData> weatherTypeDatas;
    
    std::vector<WeatherParticle> weatherParticles;

    WeatherType currentWeatherType;

    static constexpr float PARTICLE_SPAWN_RATE = 0.001f;
    float particleSpawnTimer;

};