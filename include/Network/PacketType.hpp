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

    PlayerData,
    PlayerCharacterInfo,
    WorldInfo,
    RoomDestInfo,
    ServerInfo,

    ObjectHit,
    ObjectBuilt,
    ObjectDestroyed,

    ItemPickupsCreated,
    ItemPickupCollected,
    ItemPickupsCreateRequest,
    InventoryAddItem,

    ObjectInteract,
    ChestOpened,
    ChestClosed,
    ChestDataModified,

    ChunkDatas,
    ChunkRequests,

    Entities,
    Projectiles,
    Bosses,

    ProjectileCreateRequest,

    StructureEnterRequest,
    StructureEnterReply,

    PlanetTravelRequest,
    PlanetTravelReply,
    RoomTravelRequest,
    RoomTravelReply,
};