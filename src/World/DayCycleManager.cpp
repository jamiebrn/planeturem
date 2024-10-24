#include "World/DayCycleManager.hpp"

void DayCycleManager::update(float dt)
{
    currentTime += dt;

    if (currentTime >= DAY_LENGTH)
    {
        currentTime -= DAY_LENGTH;
        currentDay++;
    }
}

bool DayCycleManager::isDay()
{
    static constexpr float DAY_LIGHT_THRESHOLD = 0.3f;
    return (getLightLevel() >= DAY_LIGHT_THRESHOLD);
}

float DayCycleManager::getLightLevel()
{
    static constexpr float a = 13.4f;
    static constexpr float b = 1.5f;
    static constexpr float c = 2.106f;
    static constexpr float d = 0.05f;

    float dayProgress = currentTime / DAY_LENGTH;

    float light = (std::sqrt((1.0f + a * a) / (1.0f + a * a * std::pow(std::cos((dayProgress - b) * 2.0f * M_PI), 2.0f))) *
        std::cos((dayProgress - b) * 2.0f * M_PI) + 1.0f) * (1.0f / c) + d;
    
    return light;
}

float DayCycleManager::getCurrentTime()
{
    return currentTime;
}

void DayCycleManager::setCurrentTime(float time)
{
    currentTime = time;
}

std::string DayCycleManager::getTimeString()
{
    float progress = currentTime / DAY_LENGTH;
    int hours = std::floor(progress * 24);
    int minutes = static_cast<int>(std::floor(progress * 24 * 60)) % 60;

    return std::to_string(hours) + ":" + std::to_string(minutes);
}

int DayCycleManager::getCurrentDay()
{
    return currentDay;
}

void DayCycleManager::setCurrentDay(int day)
{
    currentDay = day;
}

float DayCycleManager::getDayLength()
{
    return DAY_LENGTH;
}