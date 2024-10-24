#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <string>

class DayCycleManager
{
public:
    DayCycleManager() = default;

    void update(float dt);

    bool isDay();

    float getLightLevel();

    float getCurrentTime();
    void setCurrentTime(float time);
    std::string getTimeString();

    int getCurrentDay();
    void setCurrentDay(int day);

    float getDayLength();

private:
    int currentDay = 0;

    static constexpr float DAY_LENGTH = 12 * 60; // length for day
    float currentTime = 0.5f;

};