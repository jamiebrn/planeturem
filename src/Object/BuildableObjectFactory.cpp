#include "Object/BuildableObjectFactory.hpp"
#include "World/ChunkManager.hpp"

namespace BuildableObjectFactory
{

#define OBJECT_CREATE(type, condition, ...) if (condition) {return std::make_unique<type>(__VA_ARGS__);}

std::unique_ptr<BuildableObject> create(pl::Vector2f position, ObjectType objectType, const BuildableObjectCreateParameters& parameters,
    Game* game, ChunkManager* chunkManager)
{
    if (objectType >= 0)
    {
        const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

        OBJECT_CREATE(ChestObject, objectData.chestCapacity > 0,
            position, objectType, parameters)
        OBJECT_CREATE(RocketObject, objectData.rocketObjectData.has_value(),
            position, objectType, parameters)
        OBJECT_CREATE(PlantObject, objectData.plantStageObjectData.has_value() && game,
            position, objectType, parameters, *game, chunkManager)
        OBJECT_CREATE(NPCObject, objectData.npcObjectData.has_value(),
            position, objectType, parameters)
        OBJECT_CREATE(LandmarkObject, objectData.isLandmark && game && chunkManager,
            position, objectType, chunkManager->getPlanetType(), *game, parameters)
        OBJECT_CREATE(SpawnPointObject, objectData.setSpawnPoint,
            position, objectType, parameters)
    }

    // Default case
    return std::make_unique<BuildableObject>(position, objectType, parameters);
}

#undef OBJECT_CREATE

}