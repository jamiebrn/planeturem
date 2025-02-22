#pragma once

enum class PacketType : uint8_t
{
    JoinQuery,
    JoinReply,

    JoinInfo,
    PlayerJoined,
    PlayerDisconnected,
    HostQuit,

    PlayerInfo,
    WorldInfo,

    ObjectHit,
    ObjectBuilt,
    ObjectDestroyed,

    ItemPickupsCreated,
    ItemPickupDeleted,
    InventoryAddItem,

    ChunkDatas,
    ChunkRequests,
};