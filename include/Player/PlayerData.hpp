#pragma once

#include <string>
#include <sstream>
#include <unordered_set>
#include <unordered_map>

#include <extlib/cereal/types/unordered_set.hpp>
#include <extlib/cereal/types/unordered_map.hpp>
#include <extlib/cereal/types/string.hpp>
#include <extlib/cereal/archives/binary.hpp>

#include <Core/json.hpp>

#include <Graphics/Color.hpp>
#include <Vector.hpp>

#include "Data/typedefs.hpp"
#include "Data/Serialise/Vector2Serialise.hpp"
#include "Data/Serialise/ColorSerialise.hpp"
#include "Data/RecipeDataLoader.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"
#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"
#include "Player/InventoryData.hpp"
#include "Player/LocationState.hpp"
#include "Object/ObjectReference.hpp"

struct PlayerData
{
    InventoryData inventory;
    InventoryData armourInventory;
    
    // Index of recipe
    std::unordered_set<uint64_t> recipesSeen;
    
    std::unordered_map<PlanetType, ObjectReference> planetRocketUsedPositions;
    ObjectType lastUsedPlanetRocketType;
    
    std::string name;
    
    pl::Color bodyColor = pl::Color(158, 69, 57, 255);
    pl::Color skinColor = pl::Color(230, 144, 78, 255);

    pl::Vector2f position;
    
    LocationState locationState;
    pl::Vector2f structureExitPos; // used to determine structure exit position on planet
    
    int maxHealth = 0;

    // Do not save
    std::string pingLocation;

    template <class Archive>
    void serialize(Archive& ar, uint32_t version)
    {
        ar(inventory, armourInventory, recipesSeen, planetRocketUsedPositions, lastUsedPlanetRocketType, name, bodyColor, skinColor, position.x, position.y,
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

    if (json.contains("body-color"))
    {
        data.bodyColor = json.at("body-color");
    }

    if (json.contains("skin-color"))
    {
        data.skinColor = json.at("skin-color");
    }

    if (json.contains("structure-exit-pos"))
    {
        data.structureExitPos = json.at("structure-exit-pos");
    }

    for (const std::string& recipeHashStr : json["recipes-seen"])
    {
        std::stringstream stream;
        stream << std::hex << std::stoull(recipeHashStr, nullptr, 16);
        uint64_t recipeHash;
        stream >> recipeHash;

        if (RecipeDataLoader::recipeHashExists(recipeHash))
        {
            data.recipesSeen.insert(recipeHash);
        }
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

    if (json.contains("last-used-planet-rocket-type"))
    {
        data.lastUsedPlanetRocketType = ObjectDataLoader::getObjectTypeFromName(json.at("last-used-planet-rocket-type"));
    }
    else
    {
        data.lastUsedPlanetRocketType = ObjectDataLoader::getObjectTypeFromName("Rocket Launch Pad");
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

    json["body-color"] = data.bodyColor;
    json["skin-color"] = data.skinColor;

    if (data.locationState.isInStructure())
    {
        json["structure-exit-pos"] = data.structureExitPos;
    }

    std::vector<std::string> recipesSeenHashes;
    for (uint64_t recipeHash : data.recipesSeen)
    {
        std::stringstream stream;
        stream << std::hex << recipeHash;
        recipesSeenHashes.push_back(stream.str());
    }
    json["recipes-seen"] = recipesSeenHashes;

    for (const auto& rocketUsed : data.planetRocketUsedPositions)
    {
        const PlanetGenData& planetData = PlanetGenDataLoader::getPlanetGenData(rocketUsed.first);
        std::vector<int> objectReference = {rocketUsed.second.chunk.x, rocketUsed.second.chunk.y, rocketUsed.second.tile.x, rocketUsed.second.tile.y};
        json["rockets-used"][planetData.name] = objectReference;
    }

    if (data.lastUsedPlanetRocketType >= 0)
    {
        json["last-used-planet-rocket-type"] = ObjectDataLoader::getObjectData(data.lastUsedPlanetRocketType).name;
    }
}