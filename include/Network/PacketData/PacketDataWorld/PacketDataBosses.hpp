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

#include "Entity/Boss/BossManager.hpp"

struct PacketDataBosses : public IPacketData, public IPacketTimeDependent
{
    float pingTime;

    uint8_t planetType;
    BossManager bossManager;

    inline virtual void applyPingCorrection(float pingTimeSecs) override
    {
        pingTime = pingTimeSecs;
    }

    template <class Archive>
    void save(Archive& ar) const
    {
        bool hasBosses = (bossManager.getBossCount() > 0);
        ar(hasBosses, planetType);

        if (hasBosses)
        {
            ar(bossManager);
        }
    }

    template <class Archive>
    void load(Archive& ar)
    {
        bool hasBosses;
        ar(hasBosses, planetType);

        if (hasBosses)
        {
            ar(bossManager);
        }
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const override
    {
        return PacketType::Bosses;
    }
};