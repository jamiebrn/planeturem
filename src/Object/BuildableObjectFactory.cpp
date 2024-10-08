#include "Object/BuildableObjectFactory.hpp"

std::unique_ptr<BuildableObject> BuildableObjectFactory::create(sf::Vector2f position, ObjectType objectType)
{
    if (objectType > 0)
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
    }

    // Default case
    return std::make_unique<BuildableObject>(position, objectType);
}