#pragma once

#include <algorithm>
#include <cmath>
#include <random>

#include <SFML/System/Vector2.hpp>

namespace Helper
{

inline float lerp(float start, float dest, float weight)
{
    return std::min(std::max(start + weight * (dest - start), std::min(dest, start)), std::max(dest, start));
}

// High is inclusive
inline int randInt(int low, int high, long unsigned int seed = 0)
{
    if (seed == 0)
    {
        seed = time(NULL);
    }

    std::default_random_engine gen(seed);
    std::uniform_int_distribution<int> dist(low, high);

    return dist(gen);
}

// Angle in radians
inline sf::Vector2f rotateVector(sf::Vector2f vector, float angle)
{
    sf::Vector2f rotatedVector;
    rotatedVector.x = vector.x * std::cos(angle) - vector.y * std::sin(angle);
    rotatedVector.y = vector.x * std::sin(angle) + vector.y * std::cos(angle);

    return rotatedVector;
}

}