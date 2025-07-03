#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

#include <Vector.hpp>
#include <Rect.hpp>

#include "Data/typedefs.hpp"
#include "Data/ItemDrop.hpp"

struct PlantStageObjectData
{
    std::vector<pl::Rect<int>> textureRects;
    pl::Vector2f textureOrigin;
    int health;

    int minDay = 0;
    int maxDay = 0;

    std::vector<ItemDrop> itemDrops;
};

struct RocketObjectData
{
    pl::Rect<int> textureRect;
    pl::Vector2f textureOrigin;
    pl::Vector2f launchPosition;

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
    pl::Vector2<int> portraitTextureOffset;
    std::vector<std::string> dialogueLines;
    
    std::vector<ItemCount> shopItems;
    std::unordered_map<ItemType, float> buyPriceMults;
    std::unordered_map<ItemType, float> sellPriceMults;
};

struct ObjectData
{
    std::string name;
    int health = 1;
    
    std::vector<pl::Rect<int>> textureRects;
    float textureFrameDelay = 0.0f;

    pl::Vector2f textureOrigin;

    pl::Vector2<int> size = {1, 1};

    std::vector<float> lightEmissionFrames;
    float lightAbsorption = 0.0f;

    bool hasCollision = false;
    bool placeOnWater = false;

    int drawLayer = 0;

    std::string craftingStation = "";
    int craftingStationLevel = 0;

    int chestCapacity = 0;

    int minimumDamage = 1;

    // std::unordered_map<ItemType, > itemDrops;
    std::vector<ItemDrop> itemDrops;

    // If is rocket / launch pad
    std::optional<RocketObjectData> rocketObjectData = std::nullopt;

    // If is a plant
    std::optional<std::vector<PlantStageObjectData>> plantStageObjectData = std::nullopt;

    // If is NPC
    std::optional<NPCObjectData> npcObjectData = std::nullopt;

    // If is landmark
    bool isLandmark = false;

    // If is bed / spawn point
    bool setSpawnPoint = false;

    bool mythicalItem = false;
};