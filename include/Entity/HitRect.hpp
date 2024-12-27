#pragma once

#include "Core/CollisionRect.hpp"
#include "Core/CollisionCircle.hpp"

struct HitRect : public CollisionRect
{
    float damage;
};

struct HitCircle : public CollisionCircle
{
    float damage;
};