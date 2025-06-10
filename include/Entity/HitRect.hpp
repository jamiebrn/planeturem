#pragma once

#include "Core/CollisionRect.hpp"
#include "Core/CollisionCircle.hpp"

struct HitRect : public CollisionRect
{
    float damage;
    
    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(cereal::base_class<CollisionRect>(this), damage);
    }
};

struct HitCircle : public CollisionCircle
{
    float damage;
};