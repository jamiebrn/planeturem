#pragma once

#include <optional>

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"
#include "Object/ObjectReference.hpp"

struct PacketDataObjectBuilt : public IPacketData
{
    PlanetType planetType;
    ObjectReference objectReference;
    ObjectType objectType;
    std::optional<uint64_t> userId;
    bool builtByPlayer = true;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(planetType, objectReference, objectType, userId, builtByPlayer);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::ObjectBuilt;
    }
};