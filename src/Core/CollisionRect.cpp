#include "Core/CollisionRect.hpp"
#include "Core/CollisionCircle.hpp"

CollisionRect::CollisionRect(float x, float y, float width, float height)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
}

void CollisionRect::translateAroundWorld(float xOrigin, float yOrigin, int worldSize)
{
    if (worldSize <= 0)
    {
        return;
    }

    pl::Vector2f translatedPos = Camera::translateWorldPos(pl::Vector2f(x, y), pl::Vector2f(xOrigin, yOrigin), worldSize);
    x = translatedPos.x;
    y = translatedPos.y;
}

bool CollisionRect::handleCollision(CollisionRect otherRect, int worldSize)
{
    otherRect.translateAroundWorld(x, y, worldSize);

    // If not colliding with other rectangle, return false
    if (!isColliding(otherRect))
        return false;
    
    // Calculate rect intersection
    float xIntersect = (x + width) - otherRect.x;
    float yIntersect = (y + height) - otherRect.y;

    if (x > otherRect.x) xIntersect = otherRect.x + otherRect.width - x;
    if (y > otherRect.y) yIntersect = otherRect.y + otherRect.height - y;

    // Apply push to handle collision
    if (std::fabs(xIntersect) <= std::fabs(yIntersect))
    {
        if (x < otherRect.x) x -= xIntersect;
        else x += xIntersect;
    }
    else
    {
        if (y < otherRect.y) y -= yIntersect;
        else y += yIntersect;
    }
    
    // Return true as collision has occured
    return true;
}

bool CollisionRect::handleStaticCollisionX(CollisionRect staticRect, float dx, int worldSize)
{
    staticRect.translateAroundWorld(x, y, worldSize);

    if (!isColliding(staticRect))
        return false;
    
    // Test both x directions
    if (dx > 0) x = staticRect.x - width;
    else if (dx < 0) x = staticRect.x + staticRect.width;
    
    // Collision has occured
    return true;
}

bool CollisionRect::handleStaticCollisionY(CollisionRect staticRect, float dy, int worldSize)
{
    staticRect.translateAroundWorld(x, y, worldSize);

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
    
bool CollisionRect::isColliding(CollisionRect otherRect, int worldSize) const
{
    otherRect.translateAroundWorld(x, y, worldSize);
    return isColliding(otherRect);
}

bool CollisionRect::isColliding(CollisionCircle circle, int worldSize) const
{
    if (worldSize > 0)
    {
        pl::Vector2f translatedPos = Camera::translateWorldPos(pl::Vector2f(circle.x, circle.y), pl::Vector2f(x, y), worldSize);
        circle.x = translatedPos.x;
        circle.y = translatedPos.y;
    }

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

void CollisionRect::debugDraw(pl::RenderTarget& window, const Camera& camera, int worldSize, pl::Color color) const
{
    float scale = ResolutionHandler::getScale();

    pl::VertexArray rect;
    rect.addQuad(pl::Rect<float>(camera.worldToScreenTransform(pl::Vector2f(x, y), worldSize), pl::Vector2f(width, height) * scale), color, pl::Rect<float>());

    window.draw(rect, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);
}