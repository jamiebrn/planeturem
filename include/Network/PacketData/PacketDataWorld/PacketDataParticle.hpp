#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Object/ParticleSystem.hpp"

struct PacketDataParticle : public IPacketData
{
    uint8_t planetType;
    Particle particle;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(planetType, particle);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const override
    {
        return PacketType::Particle;
    }
};