#pragma once

#include <vector>
#include <array>
#include <optional>
#include <unordered_map>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/array.hpp>
#include <extlib/cereal/types/optional.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/string.hpp>

#include "Network/IPacketData.hpp"
#include "Network/IPacketTimeDependent.hpp"

#include "Data/typedefs.hpp"

#include "Entity/Projectile/Projectile.hpp"

struct PacketDataProjectileCreateRequest : public IPacketData, public IPacketTimeDependent
{
    float pingTime;

    uint8_t planetType;
    Projectile projectile;

    inline virtual void applyPingCorrection(float pingTimeSecs) override
    {
        pingTime = pingTimeSecs;
    }

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(planetType, projectile);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const
    {
        return PacketType::ProjectileCreateRequest;
    }
};