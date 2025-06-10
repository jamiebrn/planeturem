#include "Player/LocationState.hpp"

void LocationState::setToNull()
{
    currentPlanetType = -1;
    currentRoomDestType = -1;
    inStructureID = std::nullopt;
}

void LocationState::setPlanetType(PlanetType planetType)
{
    setToNull();
    currentPlanetType = planetType;
}

void LocationState::setInStructureID(std::optional<uint32_t> structureID)
{
    inStructureID = structureID;
}

void LocationState::setRoomDestType(RoomType roomType)
{
    setToNull();
    currentRoomDestType = roomType;
}

PlanetType LocationState::getPlanetType() const
{
    return currentPlanetType;
}

uint32_t LocationState::getInStructureID() const
{
    if (!inStructureID.has_value())
    {
        return 0xFFFFFFFF;
    }

    return inStructureID.value();
}

RoomType LocationState::getRoomDestType() const
{
    return currentRoomDestType;
}

bool LocationState::isNull() const
{
    return (currentPlanetType < 0 && currentRoomDestType < 0);
}

bool LocationState::isOnPlanet() const
{
    return (currentPlanetType >= 0);
}

bool LocationState::isInRoom() const
{
    return (isInStructure() || isInRoomDest());
}

bool LocationState::isInStructure() const
{
    return (isOnPlanet() && inStructureID.has_value());
}

bool LocationState::isInRoomDest() const
{
    return (currentRoomDestType >= 0);
}

GameState LocationState::getGameState() const
{
    if (isOnPlanet())
    {
        if (isInStructure())
        {
            return GameState::InStructure;
        }
        return GameState::OnPlanet;
    }
    return GameState::InRoomDestination;
}

LocationState LocationState::createFromPlanetType(PlanetType planetType)
{
    LocationState locationState;
    locationState.setPlanetType(planetType);
    return locationState;
}

LocationState LocationState::createFromStructureID(PlanetType planetType, uint32_t structureID)
{
    LocationState locationState;
    locationState.setPlanetType(planetType);
    locationState.setInStructureID(structureID);
    return locationState;
}

LocationState LocationState::createFromRoomDestType(RoomType roomType)
{
    LocationState locationState;
    locationState.setRoomDestType(roomType);
    return locationState;
}

void from_json(const nlohmann::json& json, LocationState& locationState)
{
    if (json.contains("planet"))
    {
        locationState.setPlanetType(PlanetGenDataLoader::getPlanetTypeFromName(json["planet"]));
        if (json.contains("in-structure-id"))
        {
            locationState.setInStructureID(json["in-structure-id"]);
        }
    }
    else if (json.contains("roomdest"))
    {
        locationState.setRoomDestType(StructureDataLoader::getRoomTypeTravelLocationFromName(json["roomdest"]));
    }
    else
    {
        const std::string& defaultPlanetName = PlanetGenDataLoader::getPlanetGenData(0).name;
        std::cout << "ERROR: Loaded LocationState has no previous location. Defaulting to planet \"" + defaultPlanetName + "\"\n";
        locationState.setPlanetType(0);
    }
}

void to_json(nlohmann::json& json, const LocationState& locationState)
{
    if (locationState.isOnPlanet())
    {
        json["planet"] = PlanetGenDataLoader::getPlanetGenData(locationState.getPlanetType()).name;
        if (locationState.isInStructure())
        {
            json["in-structure-id"] = locationState.getInStructureID();
        }
    }
    else if (locationState.isInRoomDest())
    {
        json["roomdest"] = StructureDataLoader::getRoomData(locationState.getRoomDestType()).name;
    }
}