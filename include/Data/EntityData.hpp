#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/CollisionRect.hpp"
#include "Data/typedefs.hpp"
#include "Data/ItemDrop.hpp"

struct EntityData
{
    std::string name;
    int health;

    std::vector<pl::Rect<int>> idleTextureRects;
    std::vector<pl::Rect<int>> walkTextureRects;
    float idleAnimSpeed;
    float walkAnimSpeed;

    pl::Vector2f textureOrigin;

    // Size of entity IN TILES
    // e.g. (1, 1) size means screen size is (tileSize * scale, tileSize * scale)
    pl::Vector2f size;

    std::vector<ItemDrop> itemDrops;
    CollisionRect hitCollision;

    std::string behaviour;
    std::unordered_map<std::string, float> behaviourParameters;
};