#pragma once

#include <SFML/Graphics.hpp>

#include <vector>

#include "Object/WorldObject.hpp"

class WeatherParticle : public WorldObject
{
private:
    sf::Vector2f velocity;
};

class WeatherSystem
{
public:
    WeatherSystem() = default;

    std::vector<WorldObject*> getWeatherParticles();

    float getLightRedBias();
    float getLightGreenBias();
    float getLightBlueBias();

private:
    std::vector<WeatherParticle> weatherParticles;
    
    float redLightBias = 0.0f, greenLightBias = 0.0f, blueLightBias = 0.0f;

};