#pragma once

#include <SFML/Graphics.hpp>
#include <string>
// #include <unordered_map>
#include <vector>
#include <optional>

#include "Data/typedefs.hpp"
#include "Data/ItemDrop.hpp"

struct RocketObjectData
{
    sf::IntRect textureRect;
    sf::Vector2f textureOrigin;
    sf::Vector2f launchPosition;
};

struct ObjectData
{
    std::string name;
    int health;
    
    std::vector<sf::IntRect> textureRects;
    float textureFrameDelay = 0.0f;

    sf::Vector2f textureOrigin;

    sf::Vector2i size = {1, 1};

    bool hasCollision = false;
    bool placeOnWater = false;

    int drawLayer = 0;

    std::string craftingStation = "";
    int craftingStationLevel = 0;

    int chestCapacity = 0;

    // std::unordered_map<ItemType, > itemDrops;
    std::vector<ItemDrop> itemDrops;

    // If is rocket / launch pad
    std::optional<RocketObjectData> rocketObjectData = std::nullopt;

    bool mythicalItem = false;
};