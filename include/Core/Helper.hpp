#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <format>

#include <Vector.hpp>

namespace Helper
{

inline float lerp(float start, float dest, float weight)
{
    return std::min(std::max(start + weight * (dest - start), std::min(dest, start)), std::max(dest, start));
}

// High is inclusive
inline int randInt(int low, int high)
{
    return rand() % (high + 1 - low) + low;
}

inline float randFloat(float low, float high)
{
    float t = randInt(0, 10000) / 10000.0f;
    return (high - low) * t + low;
}

inline float roundTo(float number, int decimalPoints)
{
    int exp = std::pow(10, decimalPoints);
    return std::round(number * exp) / exp;
}

inline std::string floatToString(float value, int decimalPlaces)
{
    return std::format("{:.{}f}", value, decimalPlaces);
}

template <typename T>
inline T sign(T value)
{
    if (value > 0)
    {
        return 1;
    }
    else if (value < 0)
    {
        return -1;
    }
    return 0;
}

inline float step(float start, float dest, float maxStep)
{
    return std::clamp(start + sign(dest - start) * maxStep, std::min(start, dest), std::max(start, dest));
}

inline int wrap(int value, int max)
{
    return (value % max + max) % max;
};

// Angle in radians
inline pl::Vector2f rotateVector(pl::Vector2f vector, float angle)
{
    pl::Vector2f rotatedVector;
    rotatedVector.x = vector.x * std::cos(angle) - vector.y * std::sin(angle);
    rotatedVector.y = vector.x * std::sin(angle) + vector.y * std::cos(angle);

    return rotatedVector;
}

inline float getVectorLength(pl::Vector2f vector)
{
    return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

inline pl::Vector2f normaliseVector(pl::Vector2f vector)
{
    float length = getVectorLength(vector);
    if (length > 0)
    {
        return vector / length;
    }
    return pl::Vector2f(0, 0);
}

}