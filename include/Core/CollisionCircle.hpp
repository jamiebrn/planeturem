#pragma once

#include <cmath>
#include <SFML/Graphics.hpp>

#include "Core/Helper.hpp"
#include "Core/Camera.hpp"

struct CollisionRect;

struct CollisionCircle
{
    float x = 0.0f, y = 0.0f;
    float radius = 1.0f;

    CollisionCircle() = default;
    CollisionCircle(float x, float y, float radius);

    bool isColliding(const CollisionCircle& otherCircle) const;
    bool isColliding(const CollisionRect& rect) const;
    bool isPointColliding(float x, float y) const;

    void debugDraw(sf::RenderTarget& window, const Camera& camera, sf::Color color = {255, 0, 0, 120}) const;
};