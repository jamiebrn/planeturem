#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <vector>

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

    sf::IntRect textureRect;

    sf::Vector2f origin;

    int damageLow = 0;
    int damageHigh = 0;

    float speed = 0.0f;

    float collisionRadius = 1.0f;
    sf::Vector2f collisionOffset;

    // Double link with item data, as projectiles are required to be dynamically taken from inventory
    // based on projectile type, not item type
    ItemType itemType;
};

struct ToolData
{
    std::string name;

    ToolBehaviourType toolBehaviourType;

    std::vector<sf::IntRect> textureRects;
    sf::Vector2f pivot;
    sf::Vector2i holdOffset;

    int damage = 0;
    
    // Relative to pivot, pixel offset (can be float) to line origin
    sf::Vector2f fishingRodLineOffset;
    float fishingEfficiency = 0.0f;

    // Bow weapon stuff
    std::vector<ProjectileType> projectileShootTypes;
    float shootPower = 1.0f;
    float projectileDamageMult = 1.0f;
    sf::Vector2i shootOffset;

    // Melee weapon stuff
    
};