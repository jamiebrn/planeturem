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
    
    ChatMessage,

    ObjectHit,
    ObjectBuilt,
    ObjectDestroyed,

    LandPlaced,

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
    ChunkModifiedAlerts,

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