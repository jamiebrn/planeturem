#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "World/LandmarkManager.hpp"

struct PacketDataLandmarks : public IPacketData
{
    uint8_t planetType;
    LandmarkManager landmarkManager;
    
    template <class Archive>
    void save(Archive& ar) const
    {
        bool hasLandmarks = (landmarkManager.getLandmarkCount() > 0);
        ar(hasLandmarks, planetType);

        if (hasLandmarks)
        {
            ar(landmarkManager);
        }
    }

    template <class Archive>
    void load(Archive& ar)
    {
        bool hasLandmarks;
        ar(hasLandmarks, planetType);

        if (hasLandmarks)
        {
            ar(landmarkManager);
        }
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const override
    {
        return PacketType::Landmarks;
    }
};