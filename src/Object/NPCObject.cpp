#include "Object/NPCObject.hpp"
#include "Game.hpp"

NPCObject::NPCObject(sf::Vector2f position, ObjectType objectType)
    : BuildableObject(position, objectType, false)
{

}

BuildableObject* NPCObject::clone()
{
    return new NPCObject(*this);
}

void NPCObject::update(Game& game, float dt, bool onWater, bool loopAnimation)
{
    BuildableObject::update(game, dt, onWater, true);
}

bool NPCObject::damage(int amount, Game& game, InventoryData& inventory, bool giveItems)
{
    // Cannot damage NPC
    return false;
}

void NPCObject::interact(Game& game)
{
    game.interactWithNPC(*this);
}

bool NPCObject::isInteractable() const
{
    return true;
}

void NPCObject::triggerBehaviour(Game& game, ObjectBehaviourTrigger trigger)
{

}

const NPCObjectData& NPCObject::getNPCObjectData() const
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    assert(objectData.npcObjectData.has_value());

    return objectData.npcObjectData.value();
}