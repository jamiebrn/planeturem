#include "Core/CollisionRect.hpp"

CollisionRect::CollisionRect(float x, float y, float width, float height)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
}

bool CollisionRect::handleCollision(const CollisionRect& otherRect)
{
    // If not colliding with other rectangle, return false
    if (!(x < otherRect.x + otherRect.width &&
    x + width > otherRect.x &&
    y < otherRect.y + otherRect.height &&
    y + height > otherRect.y))
        return false;
    
    // Calculate rect intersection
    float x_intersect = (x + width) - otherRect.x;
    float y_intersect = (y + height) - otherRect.y;

    if (x > otherRect.x) x_intersect = otherRect.x + otherRect.width - x;
    if (y > otherRect.y) y_intersect = otherRect.y + otherRect.height - y;

    // Apply push to handle collision
    if (fabs(x_intersect) <= fabs(y_intersect))
    {
        if (x < otherRect.x) x -= x_intersect;
        else x += x_intersect;
    }
    else
    {
        if (y < otherRect.y) y -= y_intersect;
        else y += y_intersect;
    }
    
    // Return true as collision has occured
    return true;
}