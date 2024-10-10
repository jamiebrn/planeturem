#include "Object/ChestObject.hpp"
#include "Game.hpp"

ChestObject::ChestObject(sf::Vector2f position, ObjectType objectType)
    : BuildableObject(position, objectType, false)
{
    closeChest();
}

BuildableObject* ChestObject::clone()
{
    return new ChestObject(*this);
}

void ChestObject::update(Game& game, float dt, bool onWater, bool loopAnimation)
{
    BuildableObject::update(game, dt, onWater, false);

    if (!isAlive())
    {
        removeChestFromPool(game);
    }

    if (game.getOpenChestID() != chestID)
    {
        closeChest();
    }
}

void ChestObject::interact(Game& game)
{
    // Chest interaction stuff
    std::cout << "interacted with chest\n";

    if (chestID == 0xFFFF)
    {
        const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
        chestID = game.getChestDataPool().createChest(objectData.chestCapacity);
    }
    else
    {
        // Close chest if already open
        if (game.getOpenChestID() == chestID)
        {
            game.closeChest();
            return;
        }
    }

    // If chestID is still 0xFFFF, then max chest number has been reached
    if (chestID == 0xFFFF)
    {
        return;
    }

    // Animation
    openChest();

    game.openChest(*this);
}

bool ChestObject::isInteractable() const
{
    return true;
}

void ChestObject::triggerBehaviour(Game& game, ObjectBehaviourTrigger trigger)
{

}

// int ChestObject::getChestCapactity()
// {
//     if (objectType < 0)
//         return 0;

//     const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

//     return objectData.chestCapacity;
// }

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

void ChestObject::removeChestFromPool(Game& game)
{
    std::cout << "removed chest " << chestID << " from pool\n";
    game.getChestDataPool().destroyChest(chestID);
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