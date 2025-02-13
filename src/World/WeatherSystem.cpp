#include "World/WeatherSystem.hpp"

const std::unordered_map<WeatherType, WeatherTypeData> WeatherSystem::weatherTypeDatas = {
    {WeatherType::Rain, {}}
};

std::vector<WorldObject*> WeatherSystem::getWeatherParticles()
{
    std::vector<WorldObject*> particles;
    for (WeatherParticle& weatherParticle : weatherParticles)
    {
        particles.push_back(&weatherParticle);
    }
    return particles;
}

const WeatherTypeData& WeatherSystem::getWeatherTypeData() const
{
    return weatherTypeDatas.at(currentWeatherType);
}