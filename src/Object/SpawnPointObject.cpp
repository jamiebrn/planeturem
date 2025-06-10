#include "Object/SpawnPointObject.hpp"
#include "Game.hpp"

SpawnPointObject::SpawnPointObject(pl::Vector2f position, ObjectType objectType, const BuildableObjectCreateParameters& parameters)
    : BuildableObject(position, objectType, parameters)
{

}

BuildableObject* SpawnPointObject::clone()
{
    return new SpawnPointObject(*this);
}

bool SpawnPointObject::damage(int amount, Game& game, ChunkManager& chunkManager, ParticleSystem* particleSystem, bool giveItems, bool createHitMarkers)
{
    // TODO: remove spawn point on destruction if required

    return BuildableObject::damage(amount, game, chunkManager, particleSystem, giveItems, createHitMarkers);
}

void SpawnPointObject::interact(Game& game, bool isClient)
{
    if (!game.getLocationState().isOnPlanet())
    {
        return;
    }

    PlanetType planetType =  game.getLocationState().getPlanetType();
    int worldSize = game.getChunkManager(planetType).getWorldSize();

    ObjectReference spawnLocation = getThisObjectReference(worldSize);

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    auto chunkTilePair = ChunkManager::getChunkTileFromOffset(spawnLocation.chunk, spawnLocation.tile, objectData.size.x, 0, worldSize);

    spawnLocation.chunk = chunkTilePair.first;
    spawnLocation.tile = chunkTilePair.second;

    game.setSpawnLocation(planetType, spawnLocation);
}

bool SpawnPointObject::isInteractable() const
{
    return true;
}