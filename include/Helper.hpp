#pragma once

namespace Helper
{

inline float lerp(float start, float dest, float weight)
{
    return start + weight * (dest - start);
}

}