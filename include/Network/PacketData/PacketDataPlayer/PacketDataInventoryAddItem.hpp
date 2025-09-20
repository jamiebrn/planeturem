#pragma once

#include <vector>
#include <utility>

#include <extlib/cereal/archives/binary.hpp>

#include "Network/IPacketData.hpp"

#include "Data/typedefs.hpp"

struct PacketDataInventoryAddItem : public IPacketData
{
    ItemType itemType;
    int amount;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(itemType, amount);
    }

    PACKET_SERIALISATION();
    
    inline virtual PacketType getType() const override
    {
        return PacketType::InventoryAddItem;
    }
};