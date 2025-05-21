#include "Object/BuildableObjectFactory.hpp"

namespace BuildableObjectFactory
{

#define OBJECT_CREATE(type, condition, ...) if (condition) {return std::make_unique<type>(__VA_ARGS__);}

std::unique_ptr<BuildableObject> create(pl::Vector2f position, ObjectType objectType, Game* game, bool placedByPlayer,
    bool placedByThisPlayer, ChunkManager* chunkManager, bool flash)
{
    if (objectType >= 0)
    {
        const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

        OBJECT_CREATE(ChestObject, objectData.chestCapacity > 0, position, objectType, flash)
        OBJECT_CREATE(RocketObject, objectData.rocketObjectData.has_value(), position, objectType, flash)
        OBJECT_CREATE(PlantObject, objectData.plantStageObjectData.has_value() && game, position, objectType, *game, chunkManager, !placedByPlayer, flash)
        OBJECT_CREATE(NPCObject, objectData.npcObjectData.has_value(), position, objectType, flash)
        OBJECT_CREATE(LandmarkObject, objectData.isLandmark && game, position, objectType, *game, placedByThisPlayer, flash)
    }

    // Default case
    return std::make_unique<BuildableObject>(position, objectType, true, flash);
}

#undef OBJECT_CREATE

}