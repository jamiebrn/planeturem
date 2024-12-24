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
    float distance = Helper::getVectorLength(sf::Vector2f(otherCircle.x, otherCircle.y) - sf::Vector2f(x, y));
    return (distance <= otherCircle.radius + radius);
}

bool CollisionCircle::isColliding(const CollisionRect& rect) const
{
    return rect.isColliding(*this);
}

bool CollisionCircle::isPointColliding(float x, float y) const
{
    float distance = Helper::getVectorLength(sf::Vector2f(this->x, this->y) - sf::Vector2f(x, y));
    return (distance <= radius);
}

void CollisionCircle::debugDraw(sf::RenderTarget& window, const Camera& camera, sf::Color color) const
{
    float scale = ResolutionHandler::getScale();
    sf::CircleShape circle(radius * scale);
    circle.setPosition(camera.worldToScreenTransform(sf::Vector2f(x, y)));
    circle.setOrigin(sf::Vector2f(radius * scale, radius * scale));
    circle.setFillColor(color);
    window.draw(circle);
}