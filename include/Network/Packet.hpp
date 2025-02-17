#pragma once

#include <string>

enum class PacketType
{
    JoinQuery,
    JoinReply
};

struct Packet
{
    PacketType type;
    char data[1024];
};