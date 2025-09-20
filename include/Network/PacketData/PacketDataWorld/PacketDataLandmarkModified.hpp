#pragma once

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Graphics/Color.hpp"

#include "Object/ObjectReference.hpp"

struct PacketDataLandmarkModified : public IPacketData
{
    uint8_t planetType;
    ObjectReference landmarkObjectReference;
    pl::Color newColorA;
    pl::Color newColorB;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(planetType, landmarkObjectReference, newColorA, newColorB);
    }

    PACKET_SERIALISATION();

    inline virtual PacketType getType() const override
    {
        return PacketType::LandmarkModified;
    }
};