#pragma once

#include <cmath>
#include <SFML/System/Vector2.hpp>

#include "Core/Helper.hpp"

struct CollisionCircle
{
    float x = 0.0f, y = 0.0f;
    float radius = 1.0f;

    CollisionCircle() = default;
    CollisionCircle(float x, float y, float radius);

    bool isPointColliding(float x, float y);
};