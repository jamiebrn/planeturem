#pragma once

#include "World/ChunkManager.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"
#include "Entity/Boss/BossManager.hpp"
#include "World/LandmarkManager.hpp"
#include "World/ChestDataPool.hpp"
#include "World/RoomPool.hpp"

struct RoomDestinationData
{
    Room roomDestination;
    ChestDataPool chestDataPool;
};