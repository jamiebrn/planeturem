#pragma once

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/array.hpp>
#include <extlib/cereal/types/optional.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/string.hpp>

#include "Network/IPacketData.hpp"
#include "Network/IPacketTimeDependent.hpp"

#include "Entity/Projectile/ProjectileManager.hpp"

struct PacketDataProjectiles : public IPacketData, public IPacketTimeDependent
{
    float pingTime;

    uint8_t planetType;
    ProjectileManager projectileManager;

    inline virtual void applyPingCorrection(float pingTimeSecs) override
    {
        pingTime = pingTimeSecs;
    }

    template <class Archive>
    void save(Archive& ar) const
    {
        bool hasProjectiles = (projectileManager.getProjectileCount() > 0);
        ar(hasProjectiles, planetType);

        if (hasProjectiles)
        {
            ar(projectileManager);
        }
    }

    template <class Archive>
    void load(Archive& ar)
    {
        bool hasProjectiles;
        ar(hasProjectiles, planetType);

        if (hasProjectiles)
        {
            ar(projectileManager);
        }
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const override
    {
        return PacketType::Projectiles;
    }
};