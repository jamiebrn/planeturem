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
    RoomDestInfo,
    ServerInfo,

    ObjectHit,
    ObjectBuilt,
    ObjectDestroyed,

    ItemPickupsCreated,
    ItemPickupDeleted,
    ItemPickupsCreateRequest,
    InventoryAddItem,

    ObjectInteract,
    ChestOpened,
    ChestClosed,
    ChestDataModified,

    ChunkDatas,
    ChunkRequests,

    PlanetTravelRequest,
    PlanetTravelReply,
    RoomTravelRequest,
    RoomTravelReply,
};