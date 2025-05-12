#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <format>

#include <Graphics/Color.hpp>
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

inline pl::Color convertHSVtoRGB(float h, float s, float v)
{
    float c = v * s;
    float x = c * (1.0f - std::abs(fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    float r = 0.0f, g = 0.0f, b = 0.0f;
    switch (static_cast<int>(h / 60.0f))
    {
        case 0: r = c; g = x; break;
        case 1: r = x; g = c; break;
        case 2: g = c; b = x; break;
        case 3: g = x; b = c; break;
        case 4: r = x; b = c; break;
        case 5: r = c; b = x; break;
    }

    return pl::Color((r + m) * 255.0f, (g + m) * 255.0f, (b + m) * 255.0f);
}

// Uses RGB channels in color struct for HSV respectively
inline pl::Color convertRGBtoHSV(const pl::Color& color)
{
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;

    float cmax = std::max(std::max(r, g), b);
    float cmin = std::min(std::min(r, g), b);
    float delta = cmax - cmin;

    pl::Color hsv;
    if (delta != 0)
    {
        if (cmax == r)
        {
            hsv.r = 60 * fmod((g - b) / delta, 6);
        }
        else if (cmax == g)
        {
            hsv.r = 60 * ((b - r) / delta + 2);
        }
        else if (cmax == b)
        {
            hsv.r = 60* ((r - g) / delta + 4);
        }
    }

    if (cmax != 0)
    {
        hsv.g = delta / cmax;
    }

    hsv.b = cmax;

    return hsv;
}

}