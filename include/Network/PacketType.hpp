#pragma once

enum class PacketType
{
    JoinQuery,
    JoinReply,

    JoinInfo,
    PlayerJoined,
    PlayerDisconnected,
    HostQuit,

    PlayerInfo,

    ObjectHit,
    ObjectBuilt,

    ChunkData,
};