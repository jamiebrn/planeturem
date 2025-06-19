#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "Object/ObjectReference.hpp"

struct PacketDataPlanetTravelRequest : public IPacketData
{
    PlanetType planetType;

    ObjectReference rocketUsedReference; // used when travelling from another planet, so rocket can be removed

    uint64_t userId;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(planetType, rocketUsedReference);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::PlanetTravelRequest;
    }
};