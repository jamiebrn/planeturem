#pragma once

#include <string>
#include <vector>
#include <sstream>

#include "Network/PacketType.hpp"

struct IPacketData
{
    virtual std::vector<char> serialise() const = 0;
    virtual void deserialise(const std::vector<char>& data) = 0;
    virtual PacketType getType() const = 0;
};