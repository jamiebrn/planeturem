#include "World/WeatherSystem.hpp"

std::vector<WorldObject*> WeatherSystem::getWeatherParticles()
{
    std::vector<WorldObject*> particles;
    for (WeatherParticle& weatherParticle : weatherParticles)
    {
        particles.push_back(&weatherParticle);
    }
    return particles;
}

float WeatherSystem:: getLightRedBias()
{

}

float WeatherSystem:: getLightGreenBias()
{

}

float WeatherSystem:: getLightBlueBias()
{

}