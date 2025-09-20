#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Player/LocationState.hpp"
#include "Object/ObjectReference.hpp"

struct PacketDataRocketInteraction : public IPacketData
{
    enum class InteractionType : uint8_t
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
    
    inline virtual PacketType getType() const override
    {
        return PacketType::RocketInteraction;
    }
};