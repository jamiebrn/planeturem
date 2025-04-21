#pragma once

#include <string>
#include <extlib/steam/steam_api.h>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/string.hpp>

#include "Network/IPacketData.hpp"
#include "Network/IPacketTimeDependent.hpp"
#include "Network/CompactFloat.hpp"

#include "Data/typedefs.hpp"

struct PacketDataServerInfo : public IPacketData, public IPacketTimeDependent
{
    float gameTime;
    uint16_t day;
    float time;
    
    template <class Archive>
    void save(Archive& ar) const
    {
        CompactFloat<uint16_t> timeCompact(time, 1);
        ar(gameTime, day, timeCompact);
    }

    template <class Archive>
    void load(Archive& ar)
    {
        CompactFloat<uint16_t> timeCompact;
        ar(gameTime, day, timeCompact);
        time = timeCompact.getValue(1);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const
    {
        return PacketType::ServerInfo;
    }

protected:
    inline virtual void applyPingCorrection(float pingTimeSecs) override
    {
        gameTime += pingTimeSecs;
        time += pingTimeSecs;
    }

};