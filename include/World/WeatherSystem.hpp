#pragma once

#include <SFML/Graphics.hpp>

#include <vector>
#include <unordered_map>

#include "Object/WorldObject.hpp"

enum class WeatherType
{
    Rain
};

struct WeatherTypeData
{
    std::vector<sf::IntRect> particleTextureRects;
    float redLightBias, greenLightBias, blueLightBias;
};

class WeatherParticle : public WorldObject
{
public:
    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
        const sf::Color& color) const override;

private:
    sf::Vector2f velocity;

};

class WeatherSystem
{
public:
    WeatherSystem() = default;

    std::vector<WorldObject*> getWeatherParticles();

    const WeatherTypeData& getWeatherTypeData() const;

private:
    static const std::unordered_map<WeatherType, WeatherTypeData> weatherTypeDatas;
    
    std::vector<WeatherParticle> weatherParticles;

    WeatherType currentWeatherType;

};