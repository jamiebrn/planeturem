#pragma once

#include <string>
#include <extlib/steam/steam_api.h>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/string.hpp>

#include "Network/IPacketData.hpp"
#include "Network/IPacketTimeDependent.hpp"

#include "Data/typedefs.hpp"

struct PacketDataServerInfo : public IPacketData, public IPacketTimeDependent
{
    float gameTime;
    int day;
    float time;
    
    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(gameTime, day, time);
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