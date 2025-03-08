#pragma once

#include <optional>

#include <extlib/cereal/types/optional.hpp>
#include <extlib/cereal/archives/binary.hpp>

#include <Core/json.hpp>

#include "Data/typedefs.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"
#include "Types/GameState.hpp"

class LocationState
{
public:
    void setToNull();

    void setPlanetType(PlanetType planetType);
    void setInStructureID(std::optional<uint32_t> structureID);
    void setRoomDestType(RoomType roomType);

    PlanetType getPlanetType() const;
    uint32_t getInStructureID() const;
    RoomType getRoomDestType() const;

    bool isNull() const;
    bool isOnPlanet() const;
    bool isInRoom() const;
    bool isInStructure() const;
    bool isInRoomDest() const;

    GameState getGameState() const;

    inline bool operator==(const LocationState& other) const
    {
        if (isOnPlanet() && other.isOnPlanet() && currentPlanetType == other.getPlanetType())
        {
            if (isInStructure() && other.isInStructure())
            {
                return (inStructureID.value() == other.getInStructureID());
            }
            return true;
        }
        else if (isInRoomDest() && other.isInRoomDest() && currentRoomDestType == other.getRoomDestType())
        {
            return true;
        }
        return false;
    }

    template <class Archive>
    void serialize(Archive& ar, uint32_t version)
    {
        ar(currentPlanetType, currentRoomDestType, inStructureID);
    }

private:
    PlanetType currentPlanetType = -1;
    RoomType currentRoomDestType = -1;
    std::optional<uint32_t> inStructureID;

};

CEREAL_CLASS_VERSION(LocationState, 1);

void from_json(const nlohmann::json& json, LocationState& locationState);

void to_json(nlohmann::json& json, const LocationState& locationState);