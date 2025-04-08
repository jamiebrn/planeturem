#pragma once

// #include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include <cmath>

#include <Graphics/Color.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

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
    int permanentHealthIncrease = 0;
};

struct ItemData
{
    std::string name;
    std::optional<std::string> displayName;
    std::string description;
    pl::Rect<int> textureRect;

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
    float sellValue = 0;

    // 0, 0, 0 reserved for rainbow effect
    pl::Color nameColor = pl::Color(255, 255, 255);

    inline const std::string& getDisplayName() const
    {
        if (displayName.has_value())
        {
            return displayName.value();
        }
        return name;
    }

    inline pl::Color getNameColor(float gameTime) const
    {
        if (nameColor == pl::Color(0, 0, 0))
        {
            pl::Color rainbowColor;
            rainbowColor.r = 255.0f * (std::sin(gameTime * 2.0f) + 1) / 2.0f;
            rainbowColor.g = 255.0f * (std::cos(gameTime * 2.0f) + 1) / 2.0f;
            rainbowColor.b = 255.0f * (std::sin(gameTime * 2.0f + 3 * 3.14 / 2) + 1) / 2.0f;
            return rainbowColor;
        }

        return nameColor;
    }
};