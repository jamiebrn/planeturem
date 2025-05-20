#include "Object/NPCObject.hpp"
#include "Game.hpp"

NPCObject::NPCObject(pl::Vector2f position, ObjectType objectType, bool flash)
    : BuildableObject(position, objectType, false, flash)
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

bool NPCObject::damage(int amount, Game& game, ChunkManager& chunkManager, ParticleSystem& particleSystem, bool giveItems)
{
    // Cannot damage NPC
    return false;
}

void NPCObject::interact(Game& game, bool isClient)
{
    game.interactWithNPC(*this);
}

bool NPCObject::isInteractable() const
{
    return true;
}

const NPCObjectData& NPCObject::getNPCObjectData() const
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    assert(objectData.npcObjectData.has_value());

    return objectData.npcObjectData.value();
}