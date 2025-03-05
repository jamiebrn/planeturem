#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"
#include "Network/PacketDataChunkDatas.hpp"
#include "Object/ObjectReference.hpp"

struct PacketDataPlanetTravelReply : public IPacketData
{
    PacketDataChunkDatas chunkDatas;
    ObjectReference rocketObjectReference;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(chunkDatas, rocketObjectReference);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::PlanetTravelReply;
    }
};