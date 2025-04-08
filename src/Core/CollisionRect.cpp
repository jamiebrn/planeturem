#include "Core/CollisionRect.hpp"
#include "Core/CollisionCircle.hpp"

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
    if (!isColliding(otherRect))
        return false;
    
    // Calculate rect intersection
    float x_intersect = (x + width) - otherRect.x;
    float y_intersect = (y + height) - otherRect.y;

    if (x > otherRect.x) x_intersect = otherRect.x + otherRect.width - x;
    if (y > otherRect.y) y_intersect = otherRect.y + otherRect.height - y;

    // Apply push to handle collision
    if (std::fabs(x_intersect) <= std::fabs(y_intersect))
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

bool CollisionRect::handleStaticCollisionX(const CollisionRect& staticRect, float dx)
{
    if (!isColliding(staticRect))
        return false;
    
    // Test both x directions
    if (dx > 0) x = staticRect.x - width;
    else if (dx < 0) x = staticRect.x + staticRect.width;
    
    // Collision has occured
    return true;
}

bool CollisionRect::handleStaticCollisionY(const CollisionRect& staticRect, float dy)
{
    if (!isColliding(staticRect))
        return false;
    
    // Test both y directions
    if (dy > 0) y = staticRect.y - height;
    else if (dy < 0) y = staticRect.y + staticRect.height;

    // Collision has occured
    return true;
}

bool CollisionRect::isColliding(const CollisionRect& otherRect) const
{
    return (x < otherRect.x + otherRect.width &&
    x + width > otherRect.x &&
    y < otherRect.y + otherRect.height &&
    y + height > otherRect.y);
}

bool CollisionRect::isColliding(const CollisionCircle& circle) const
{
    float testX = circle.x;
    float testY = circle.y;

    if (circle.x < x)
    {
        testX = x;
    }
    else if (circle.x > x + width)
    {
        testX = x + width;
    }

    if (circle.y < y)
    {
        testY = y;
    }
    else if (circle.y > y + height)
    {
        testY = y + height;
    }

    float distX = circle.x-testX;
    float distY = circle.y-testY;
    float distance = std::sqrt(distX * distX + distY * distY);

    return (distance <= circle.radius);
}

bool CollisionRect::isPointInRect(float x, float y) const
{
    return (this->x <= x && this->x + width >= x && this->y <= y && this->y + height >= y);
}

pl::Vector2f CollisionRect::getCentre() const
{
    return pl::Vector2f(x + width / 2, y + height / 2);
}

void CollisionRect::debugDraw(pl::RenderTarget& window, const Camera& camera, pl::Color color) const
{
    float scale = ResolutionHandler::getScale();

    pl::VertexArray rect;
    rect.addQuad(pl::Rect<float>(camera.worldToScreenTransform(pl::Vector2f(x, y)), pl::Vector2f(width, height) * scale), color.normalise(), pl::Rect<float>(0, 0, 0, 0));

    window.draw(rect, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);
}