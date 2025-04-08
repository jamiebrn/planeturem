#include "Core/CollisionCircle.hpp"
#include "Core/CollisionRect.hpp"

CollisionCircle::CollisionCircle(float x, float y, float radius)
{
    this->x = x;
    this->y = y;
    this->radius = radius;
}

bool CollisionCircle::isColliding(const CollisionCircle& otherCircle) const
{
    float distance = (pl::Vector2f(otherCircle.x, otherCircle.y) - pl::Vector2f(x, y)).getLength();
    return (distance <= otherCircle.radius + radius);
}

bool CollisionCircle::isColliding(const CollisionRect& rect) const
{
    return rect.isColliding(*this);
}

bool CollisionCircle::isPointColliding(float x, float y) const
{
    float distance = (pl::Vector2f(this->x, this->y) - pl::Vector2f(x, y)).getLength();
    return (distance <= radius);
}

void CollisionCircle::debugDraw(pl::RenderTarget& window, const Camera& camera, pl::Color color) const
{
    // float scale = ResolutionHandler::getScale();
    // sf::CircleShape circle(radius * scale);
    // circle.setPosition(camera.worldToScreenTransform(sf::Vector2f(x, y)));
    // circle.setOrigin(sf::Vector2f(radius * scale, radius * scale));
    // circle.setFillColor(color);
    // window.draw(circle);
}