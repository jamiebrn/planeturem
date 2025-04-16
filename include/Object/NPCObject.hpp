#pragma once



#include <Graphics/Color.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"

class Game;

class NPCObject : public BuildableObject
{
public:
    NPCObject(pl::Vector2f position, ObjectType objectType);

    BuildableObject* clone() override;

    void update(Game& game, float dt, bool onWater, bool loopAnimation) override;

    bool damage(int amount, Game& game, ChunkManager& chunkManager, ParticleSystem& particleSystem, bool giveItems = true) override;

    void interact(Game& game, bool isClient) override;
    bool isInteractable() const override;

    void triggerBehaviour(Game& game, ObjectBehaviourTrigger trigger) override;

    const NPCObjectData& getNPCObjectData() const;

};