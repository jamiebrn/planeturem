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

bool DayCycleManager::isDay() const
{
    static constexpr float DAY_LIGHT_THRESHOLD = 0.3f;
    return (getLightLevel() >= DAY_LIGHT_THRESHOLD);
}

float DayCycleManager::getLightLevel() const
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

float DayCycleManager::getCurrentTime() const
{
    return currentTime;
}

void DayCycleManager::setCurrentTime(float time)
{
    currentTime = time;
}

std::string DayCycleManager::getTimeString() const
{
    float progress = currentTime / DAY_LENGTH;
    int hours = std::floor(progress * 24);
    int minutes = static_cast<int>(std::floor((static_cast<int>(std::floor(progress * 24 * 60)) % 60) / 10.0f) * 10.0f);

    std::string minuteString = std::to_string(minutes);

    if (minuteString.size() < 2)
    {
        minuteString = "0" + minuteString;
    }

    return std::to_string(hours) + ":" + minuteString;
}

int DayCycleManager::getCurrentDay() const
{
    return currentDay;
}

void DayCycleManager::setCurrentDay(int day)
{
    currentDay = day;
}

std::string DayCycleManager::getDayString() const
{
    return "Day " + std::to_string(currentDay);
}

float DayCycleManager::getDayLength() const
{
    return DAY_LENGTH;
}