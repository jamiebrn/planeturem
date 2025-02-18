#pragma once

#include <string>
#include <sstream>

#include "Network/PacketType.hpp"

struct IPacketData
{
    virtual std::string serialise() const = 0;
    virtual void deserialise(const std::string& data) = 0;
    virtual PacketType getType() const = 0;
};