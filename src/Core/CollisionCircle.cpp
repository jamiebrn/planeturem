#include "Core/CollisionCircle.hpp"

CollisionCircle::CollisionCircle(float x, float y, float radius)
{
    this->x = x;
    this->y = y;
    this->radius = radius;
}

bool CollisionCircle::isPointColliding(float x, float y)
{
    float distance = Helper::getVectorLength(sf::Vector2f(this->x, this->y) - sf::Vector2f(x, y));
    return (distance <= radius);
}