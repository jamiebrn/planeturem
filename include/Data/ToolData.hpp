#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <vector>

enum ToolBehaviourType
{
    Pickaxe,
    FishingRod
};

static const std::unordered_map<std::string, ToolBehaviourType> toolBehaviourStrTypeMap = {
    {"Pickaxe", ToolBehaviourType::Pickaxe},
    {"Fishing Rod", ToolBehaviourType::FishingRod}
};

struct ToolData
{
    std::string name;

    ToolBehaviourType toolBehaviourType;

    std::vector<sf::IntRect> textureRects;
    sf::Vector2f pivot;
    sf::Vector2i holdOffset;

    // Relative to pivot, pixel offset (can be float) to line origin
    sf::Vector2f fishingRodLineOffset;

    int damage = 0;
};