#pragma once

#include <algorithm>

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

}