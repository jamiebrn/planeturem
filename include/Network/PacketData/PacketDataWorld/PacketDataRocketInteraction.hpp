#pragma once

#include <optional>

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Vector.hpp"

#include "Data/typedefs.hpp"
#include "Player/LocationState.hpp"
#include "Object/ObjectReference.hpp"

struct PacketDataRocketInteraction : public IPacketData
{
    enum class InteractionType
    {
        Enter,
        Exit,
        FlyUp,
        FlyDown
    };

    LocationState locationState;
    ObjectReference rocketObjectReference;
    InteractionType interactionType;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(locationState, rocketObjectReference, interactionType);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::RocketInteraction;
    }
};