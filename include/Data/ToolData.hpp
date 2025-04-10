#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include <Vector.hpp>
#include <Rect.hpp>

enum ToolBehaviourType
{
    Pickaxe,
    FishingRod,
    BowWeapon,
    MeleeWeapon
};

static const std::unordered_map<std::string, ToolBehaviourType> toolBehaviourStrTypeMap = {
    {"Pickaxe", ToolBehaviourType::Pickaxe},
    {"Fishing Rod", ToolBehaviourType::FishingRod},
    {"BowWeapon", ToolBehaviourType::BowWeapon},
    {"MeleeWeapon", ToolBehaviourType::MeleeWeapon}
};

struct ProjectileData
{
    std::string name;

    pl::Rect<int> textureRect;

    pl::Vector2f origin;

    int damageLow = 0;
    int damageHigh = 0;

    float speed = 0.0f;

    float collisionRadius = 1.0f;
    pl::Vector2f collisionOffset;

    // Double link with item data, as projectiles are required to be dynamically taken from inventory
    // based on projectile type, not item type
    ItemType itemType;
};

struct ToolData
{
    std::string name;

    ToolBehaviourType toolBehaviourType;

    std::vector<pl::Rect<int>> textureRects;
    pl::Vector2f pivot;
    pl::Vector2<int> holdOffset;

    int damage = 0;
    
    // Relative to pivot, pixel offset (can be float) to line origin
    pl::Vector2f fishingRodLineOffset;
    float fishingEfficiency = 0.0f;

    // Bow weapon stuff
    std::vector<ProjectileType> projectileShootTypes;
    float shootPower = 1.0f;
    float projectileDamageMult = 1.0f;
    pl::Vector2<int> shootOffset;

    // Melee weapon stuff
    
};