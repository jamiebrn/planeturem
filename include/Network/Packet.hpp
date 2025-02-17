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
    uint64_t senderSteamId;
    char data[1024];
};