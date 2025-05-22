#pragma once

#include <cstdint>

#include <Vector.hpp>

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "World/ChestDataPool.hpp"
#include "Player/LocationState.hpp"

class Game;

class ChestObject : public BuildableObject
{
public:
    ChestObject(pl::Vector2f position, ObjectType objectType, const BuildableObjectCreateParameters& parameters);

    BuildableObject* clone() override;

    void update(Game& game, float dt, bool onWater, bool loopAnimation) override;

    bool damage(int amount, Game& game, ChunkManager& chunkManager, ParticleSystem* particleSystem, bool giveItems = true, bool createHitMarkers = true) override;

    void interact(Game& game, bool isClient) override;
    bool isInteractable() const override;

    uint16_t createChestID(Game& game, std::optional<LocationState> locationState);

    inline void setChestID(uint16_t chestID) {this->chestID = chestID;}
    inline uint16_t getChestID() {return chestID;}

    // int getChestCapactity();

    void openChest();
    void closeChest();

    bool isOpen();

    // Save / load
    BuildableObjectPOD getPOD() const override;
    void loadFromPOD(const BuildableObjectPOD& pod) override;

    virtual bool injectPODMetadata(const BuildableObjectPOD& pod) override;

private:
    void removeChestFromPool(Game& game);

private:
    uint16_t chestID = 0xFFFF;

};