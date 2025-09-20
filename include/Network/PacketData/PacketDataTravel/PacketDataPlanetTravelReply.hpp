#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "World/WorldMap.hpp"

#include "Network/IPacketData.hpp"
#include "Network/PacketData/PacketDataWorld/PacketDataChunkDatas.hpp"
#include "Network/PacketData/PacketDataWorld/PacketDataLandmarks.hpp"
#include "Object/ObjectReference.hpp"

struct PacketDataPlanetTravelReply : public IPacketData
{
    PacketDataChunkDatas chunkDatas;
    PacketDataLandmarks landmarks;
    WorldMap worldMap;

    ObjectReference rocketObjectReference;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(chunkDatas, landmarks, worldMap, rocketObjectReference);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const override
    {
        return PacketType::PlanetTravelReply;
    }
};