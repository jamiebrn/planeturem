#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"
#include "Network/PacketData/PacketDataWorld/PacketDataChunkDatas.hpp"
#include "Network/PacketData/PacketDataWorld/PacketDataLandmarks.hpp"
#include "Object/ObjectReference.hpp"

struct PacketDataPlanetTravelReply : public IPacketData
{
    PacketDataChunkDatas chunkDatas;
    PacketDataLandmarks landmarks;
    ObjectReference rocketObjectReference;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(chunkDatas, landmarks, rocketObjectReference);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::PlanetTravelReply;
    }
};