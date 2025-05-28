#pragma once

#include <vector>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>

#include "Vector.hpp"

#include "Network/IPacketData.hpp"

#include "Entity/HitRect.hpp"

struct PacketDataMeleeRequest : public IPacketData
{
    uint8_t planetType;
    std::vector<HitRect> hitRects;
    pl::Vector2f hitOrigin;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(planetType, hitRects, hitOrigin.x, hitOrigin.y);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const
    {
        return PacketType::MeleeRequest;
    }
};