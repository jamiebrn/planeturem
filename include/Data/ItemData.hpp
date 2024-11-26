#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <optional>

#include "Data/typedefs.hpp"
#include "Data/ObjectData.hpp"

struct BossSummonData
{
    std::string bossName;
    bool useAtNight = false;
};

struct ConsumableData
{
    int cooldownTime;
    int healthIncrease = 0;
};

struct ItemData
{
    std::string name;
    std::string description;
    sf::IntRect textureRect;

    unsigned int maxStackSize = 99;

    ObjectType placesObjectType = -1;
    ToolType toolType = -1;
    ArmourType armourType = -1;
    ProjectileType projectileType = -1;
    bool placesLand = false;

    std::optional<BossSummonData> bossSummonData = std::nullopt;
    std::optional<ConsumableData> consumableData = std::nullopt;

    bool isMaterial = false;

    int currencyValue = 0;
    int sellValue = 0;

    // 0, 0, 0 reserved for rainbow effect
    sf::Color nameColor = sf::Color(255, 255, 255);
};