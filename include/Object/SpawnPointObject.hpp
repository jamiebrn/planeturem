#pragma once

#include <Graphics/Color.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"

class Game;

class SpawnPointObject : public BuildableObject
{
public:
    SpawnPointObject(pl::Vector2f position, ObjectType objectType, const BuildableObjectCreateParameters& parameters);

    BuildableObject* clone() override;

    bool damage(int amount, Game& game, ChunkManager& chunkManager, ParticleSystem* particleSystem, bool giveItems = true, bool createHitMarkers = true) override;

    void interact(Game& game, bool isClient) override;
    bool isInteractable() const override;

};