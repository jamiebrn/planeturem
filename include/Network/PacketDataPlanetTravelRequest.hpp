#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"

struct PacketDataPlanetTravelRequest : public IPacketData
{
    PlanetType planetType;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(planetType);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::PlanetTravelRequest;
    }
};