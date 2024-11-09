#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <string>

class DayCycleManager
{
public:
    DayCycleManager() = default;

    void update(float dt);

    bool isDay() const;

    float getLightLevel() const;

    float getCurrentTime() const;
    void setCurrentTime(float time);
    std::string getTimeString()  const;

    int getCurrentDay() const;
    void setCurrentDay(int day);
    std::string getDayString() const;

    float getDayLength() const;

private:
    int currentDay = 0;

    static constexpr float DAY_LENGTH = 12 * 60; // length for day
    float currentTime = 0.0f;

};