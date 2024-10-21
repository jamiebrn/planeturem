#pragma once

class DayCycleManager
{
public:
    DayCycleManager() = default;

    int getCurrentDay();
    void setCurrentDay(int day);

private:
    int currentDay;

    static constexpr float DAY_LENGTH = 12 * 60; // length for day
    float currentTime;

};