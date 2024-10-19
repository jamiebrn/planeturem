#pragma once

#include "Core/CollisionRect.hpp"

struct HitRect : public CollisionRect
{
    int damage;
};