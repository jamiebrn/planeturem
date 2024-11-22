#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>
#include <unordered_map>

#include "Core/json.hpp"
#include "Data/StructureData.hpp"
#include "Data/typedefs.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"

#include "Data/Serialise/Vector2Serialise.hpp"

#include "GameConstants.hpp"

class StructureDataLoader
{
    StructureDataLoader() = delete;

public:
    static bool loadData(std::string structureDataPath);

    static const StructureData& getStructureData(StructureType structureType);

    static const RoomData& getRoomData(RoomType roomType);

    static StructureType getStructureTypeFromName(const std::string& structureName);

    static RoomType getRoomTypeTravelLocationFromName(const std::string& roomLocationName);

    static inline const std::unordered_map<std::string, RoomType>& getRoomTravelLocationNameToTypeMap() {return roomTypeTravelLocationsMap;}

private:
    static void loadChestContents(RoomObjectData& roomObjectData, nlohmann::ordered_json::iterator objectIter);

    static std::vector<StructureData> loaded_structureData;
    static std::vector<RoomData> loaded_roomData;

    static std::unordered_map<std::string, StructureType> structureNameToTypeMap;

    static std::unordered_map<std::string, RoomType> roomTypeTravelLocationsMap;

};