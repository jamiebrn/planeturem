#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>

#include <extlib/cereal/archives/binary.hpp>

#include "Core/ResolutionHandler.hpp"
#include "Core/Camera.hpp"

class CollisionCircle;

struct CollisionRect
{
    float x = 0, y = 0;
    float width = 0, height = 0;

    CollisionRect() = default;
    CollisionRect(float x, float y, float width, float height);

    bool handleCollision(const CollisionRect& otherRect);
    bool handleStaticCollisionX(const CollisionRect& staticRect, float dx);
    bool handleStaticCollisionY(const CollisionRect& staticRect, float dy);

    bool isColliding(const CollisionRect& otherRect) const;
    bool isColliding(const CollisionCircle& circle) const;
    bool isPointInRect(float x, float y) const;

    void debugDraw(sf::RenderTarget& window, const Camera& camera, sf::Color color = {255, 0, 0, 120}) const;

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        ar(x, y, width, height);
    }
};

CEREAL_CLASS_VERSION(CollisionRect, 1);