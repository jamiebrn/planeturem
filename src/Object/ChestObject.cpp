#include "Object/ChestObject.hpp"

ChestObject::ChestObject(sf::Vector2f position, ObjectType objectType)
    : BuildableObject(position, objectType, false)
{
    animationDirection = -1;
}

BuildableObject* ChestObject::clone()
{
    return new ChestObject(*this);
}

void ChestObject::update(float dt, bool onWater, bool loopAnimation)
{
    BuildableObject::update(dt, onWater, false);
}

ObjectInteractionType ChestObject::interact() const
{
    return ObjectInteractionType::Chest;
}

int ChestObject::getChestCapactity()
{
    if (objectType < 0)
        return 0;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    return objectData.chestCapacity;
}

void ChestObject::openChest()
{
    // Play open chest animation
    animationDirection = 1;
}

void ChestObject::closeChest()
{
    // Rewind open chest animation
    animationDirection = -1;
}

// Save / load
BuildableObjectPOD ChestObject::getPOD() const
{
    BuildableObjectPOD pod = BuildableObject::getPOD();
    pod.chestID = chestID;
    return pod;
}

void ChestObject::loadFromPOD(const BuildableObjectPOD& pod)
{
    BuildableObject::loadFromPOD(pod);
    chestID = pod.chestID;
}