#pragma once

#include <algorithm>
#include <cmath>
#include <SFML/System/Vector2.hpp>

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

// Angle in radians
inline sf::Vector2f rotateVector(sf::Vector2f vector, float angle)
{
    sf::Vector2f rotatedVector;
    rotatedVector.x = vector.x * std::cos(angle) - vector.y * std::sin(angle);
    rotatedVector.y = vector.x * std::sin(angle) + vector.y * std::cos(angle);

    return rotatedVector;
}

inline sf::Vector2f normaliseVector(sf::Vector2f vector)
{
    float length = std::sqrt(vector.x * vector.x + vector.y * vector.y);
    return vector / length;
}

}