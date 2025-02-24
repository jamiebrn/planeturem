#pragma once

#include <cstdint>

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

    ObjectInteract,
    ChestOpened,
    ChestClosed,

    ChunkDatas,
    ChunkRequests,
};