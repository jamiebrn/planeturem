#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>

#include <extlib/cereal/types/unordered_set.hpp>
#include <extlib/cereal/types/unordered_map.hpp>
#include <extlib/cereal/types/string.hpp>
#include <extlib/cereal/archives/binary.hpp>

#include <SFML/System/Vector2.hpp>
#include <Core/json.hpp>

#include "Data/typedefs.hpp"
#include "Data/Serialise/Vector2Serialise.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"
#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"
#include "Player/InventoryData.hpp"
#include "Player/LocationState.hpp"
#include "Object/ObjectReference.hpp"

struct PlayerData
{
    // PlanetType planetType = -1;
    // uint32_t inStructureID = 0xFFFFFFFF;
    // RoomType roomDestinationType = -1;
    InventoryData inventory;
    InventoryData armourInventory;
    
    std::unordered_set<ItemType> recipesSeen;
    
    std::unordered_map<PlanetType, ObjectReference> planetRocketUsedPositions;
    
    std::string name;
    
    sf::Vector2f position;
    
    // Structure
    // bool isInStructure = false;
    
    LocationState locationState;
    sf::Vector2f structureExitPos; // used to determine structure exit position on planet
    
    int maxHealth = 0;

    // Do not save
    std::string pingLocation;

    template <class Archive>
    void serialize(Archive& ar, uint32_t version)
    {
        ar(inventory, armourInventory, recipesSeen, planetRocketUsedPositions, name, position.x, position.y,
            locationState, structureExitPos.x, structureExitPos.y, maxHealth);
    }
};

CEREAL_CLASS_VERSION(PlayerData, 1);

inline void from_json(const nlohmann::json& json, PlayerData& data)
{
    data.name = json["name"];
    data.position = json["position"];
    data.inventory = json["inventory"];
    data.armourInventory = json["armour-inventory"];
    data.maxHealth = json["max-health"];
    data.locationState = json["location-state"];

    if (json.contains("structure-exit-pos"))
    {
        data.structureExitPos = json.at("structure-exit-pos");
    }

    for (const std::string& recipeStr : json["recipes-seen"])
    {
        data.recipesSeen.insert(ItemDataLoader::getItemTypeFromName(recipeStr));
    }

    if (json.contains("rockets-used"))
    {
        for (auto iter = json["rockets-used"].begin(); iter != json["rockets-used"].end(); iter++)
        {
            ObjectReference rocketObjectReference;
            rocketObjectReference.chunk = {iter.value()[0], iter.value()[1]};
            rocketObjectReference.tile = {iter.value()[2], iter.value()[3]};
            data.planetRocketUsedPositions[PlanetGenDataLoader::getPlanetTypeFromName(iter.key())] = rocketObjectReference;
        }
    }
}

inline void to_json(nlohmann::json& json, const PlayerData& data)
{
    json["name"] = data.name;
    json["position"] = data.position;
    json["inventory"] = data.inventory;
    json["armour-inventory"] = data.armourInventory;
    json["max-health"] = data.maxHealth;
    json["location-state"] = data.locationState;

    if (data.locationState.isInStructure())
    {
        json["structure-exit-pos"] = data.structureExitPos;
    }

    std::vector<std::string> recipesSeenStrings;
    for (ItemType itemType : data.recipesSeen)
    {
        recipesSeenStrings.push_back(ItemDataLoader::getItemData(itemType).name);
    }
    json["recipes-seen"] = recipesSeenStrings;

    for (const auto& rocketUsed : data.planetRocketUsedPositions)
    {
        const PlanetGenData& planetData = PlanetGenDataLoader::getPlanetGenData(rocketUsed.first);
        std::vector<int> objectReference = {rocketUsed.second.chunk.x, rocketUsed.second.chunk.y, rocketUsed.second.tile.x, rocketUsed.second.tile.y};
        json["rockets-used"][planetData.name] = objectReference;
    }
}