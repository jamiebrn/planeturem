#pragma once

#include <cmath>
#include <algorithm>

#include <Graphics/VertexArray.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Vector.hpp>

#include <Core/Shaders.hpp>

#include <extlib/cereal/archives/binary.hpp>

#include "Core/ResolutionHandler.hpp"
#include "Core/Camera.hpp"

struct CollisionCircle;

struct CollisionRect
{
    float x = 0, y = 0;
    float width = 0, height = 0;

    CollisionRect() = default;
    CollisionRect(float x, float y, float width, float height);

    bool handleCollision(CollisionRect otherRect, int worldSize);
    bool handleStaticCollisionX(CollisionRect staticRect, float dx, int worldSize);
    bool handleStaticCollisionY(CollisionRect staticRect, float dy, int worldSize);

    void translateAroundWorld(float xOrigin, float yOrigin, int worldSize);

    bool isColliding(CollisionRect otherRect, int worldSize) const;
    // bool isColliding(const CollisionCircle& circle) const;
    bool isColliding(CollisionCircle circle, int worldSize) const;
    bool isPointInRect(float x, float y) const;
    
    pl::Vector2f getCentre() const;
    
    void debugDraw(pl::RenderTarget& window, const Camera& camera, int worldSize, pl::Color color = {255, 0, 0, 120}) const;
    
    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        ar(x, y, width, height);
    }
    
private:
    // Use when already performed world translation
    bool isColliding(const CollisionRect& otherRect) const;    

};

CEREAL_CLASS_VERSION(CollisionRect, 1);