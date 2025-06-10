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

    MeleeRequest,

    ChunkDatas,
    ChunkRequests,
    ChunkModifiedAlerts,

    Entities,
    Projectiles,
    Bosses,

    BossSpawnCheck,
    BossSpawnCheckReply,
    BossSpawnRequest,

    Landmarks,
    LandmarkModified,

    RocketEnterRequest,
    RocketEnterReply,
    RocketInteraction,

    ProjectileCreateRequest,

    StructureEnterRequest,
    StructureEnterReply,

    PlanetTravelRequest,
    PlanetTravelReply,
    RoomTravelRequest,
    RoomTravelReply,
};