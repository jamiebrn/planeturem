#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "Object/ObjectReference.hpp"

struct PacketDataObjectHit : public IPacketData
{
    PlanetType planetType;
    ObjectReference objectHit;
    int damage;
    uint64_t userId;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(planetType, objectHit, damage, userId);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const override
    {
        return PacketType::ObjectHit;
    }
};