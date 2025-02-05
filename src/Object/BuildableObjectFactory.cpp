#include "Object/BuildableObjectFactory.hpp"

namespace BuildableObjectFactory
{

std::unique_ptr<BuildableObject> create(sf::Vector2f position, ObjectType objectType, Game* game, bool placedByPlayer)
{
    if (objectType >= 0)
    {
        const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

        if (objectData.chestCapacity > 0)
        {
            return std::make_unique<ChestObject>(position, objectType);
        }
        if (objectData.rocketObjectData.has_value())
        {
            return std::make_unique<RocketObject>(position, objectType);
        }
        if (objectData.plantStageObjectData.has_value() && game)
        {
            return std::make_unique<PlantObject>(position, objectType, *game, !placedByPlayer);
        }
        if (objectData.npcObjectData.has_value())
        {
            return std::make_unique<NPCObject>(position, objectType);
        }
        if (objectData.isLandmark && game)
        {
            return std::make_unique<LandmarkObject>(position, objectType, *game, placedByPlayer);
        }
    }

    // Default case
    return std::make_unique<BuildableObject>(position, objectType);
}

}