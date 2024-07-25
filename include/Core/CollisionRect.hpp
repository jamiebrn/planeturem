#pragma once

struct CollisionRect
{
    float x, y;
    float width, height;

    CollisionRect() = default;
    CollisionRect(float x, float y, float width, float height);

    bool handleCollision(const CollisionRect& otherRect);
};