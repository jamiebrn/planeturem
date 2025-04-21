#pragma once

#include "World/ChunkManager.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"
#include "Entity/Boss/BossManager.hpp"
#include "World/LandmarkManager.hpp"
#include "World/ChestDataPool.hpp"
#include "World/RoomPool.hpp"

class Game;

struct WorldData
{
    ChunkManager chunkManager;
    ProjectileManager projectileManager;
    BossManager bossManager;
    LandmarkManager landmarkManager;
    ChestDataPool chestDataPool;
    RoomPool structureRoomPool;

    inline void initialise(Game* game, PlanetType planetType, int seed)
    {
        chunkManager.setSeed(seed);
        chunkManager.setPlanetType(planetType);

        projectileManager.initialise(game, planetType);
    }
};