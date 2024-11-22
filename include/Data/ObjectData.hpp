#pragma once

#include <SFML/Graphics.hpp>
#include <string>
// #include <unordered_map>
#include <vector>
#include <optional>

#include "Data/typedefs.hpp"
#include "Data/ItemDrop.hpp"

struct PlantStageObjectData
{
    std::vector<sf::IntRect> textureRects;
    sf::Vector2f textureOrigin;
    int health;

    int minDay = 0;
    int maxDay = 0;

    std::vector<ItemDrop> itemDrops;
};

struct RocketObjectData
{
    sf::IntRect textureRect;
    sf::Vector2f textureOrigin;
    sf::Vector2f launchPosition;

    std::vector<PlanetType> availableDestinations;
    std::vector<RoomType> availableRoomDestinations;

    // Used for loading, as planets load after objects
    // So string is stored, then planet type is loaded into available destinations
    std::vector<std::string> availableDestinationStrings;
    std::vector<std::string> availableRoomDestinationStrings;
};

struct NPCObjectData
{
    std::string npcName;
    NPCObjectBehaviour behaviour;
    sf::Vector2i portraitTextureOffset;
};

struct ObjectData
{
    std::string name;
    int health = 1;
    
    std::vector<sf::IntRect> textureRects;
    float textureFrameDelay = 0.0f;

    sf::Vector2f textureOrigin;

    sf::Vector2i size = {1, 1};

    std::vector<float> lightEmissionFrames;
    float lightAbsorption = 0.0f;

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

    // If is a plant
    std::optional<std::vector<PlantStageObjectData>> plantStageObjectData = std::nullopt;

    // If is NPC
    std::optional<NPCObjectData> npcObjectData = std::nullopt;

    bool mythicalItem = false;
};