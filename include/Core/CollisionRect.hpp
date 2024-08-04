#pragma once

#include <math.h>
#include <algorithm>

struct CollisionRect
{
    float x, y;
    float width, height;

    CollisionRect() = default;
    CollisionRect(float x, float y, float width, float height);

    bool handleCollision(const CollisionRect& otherRect);
    bool handleStaticCollisionX(const CollisionRect& staticRect, float dx);
    bool handleStaticCollisionY(const CollisionRect& staticRect, float dy);

    bool isColliding(const CollisionRect& otherRect);
    bool isPointInRect(float x, float y);
};