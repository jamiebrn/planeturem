#pragma once

#include <SFML/Graphics.hpp>

#include <cstdint>

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "World/ChestDataPool.hpp"

class Game;

class ChestObject : public BuildableObject
{
public:
    ChestObject(sf::Vector2f position, ObjectType objectType);

    BuildableObject* clone() override;

    void update(Game& game, float dt, bool onWater, bool loopAnimation) override;

    bool damage(int amount, Game& game, InventoryData& inventory) override;

    void interact(Game& game) override;
    bool isInteractable() const override;

    void triggerBehaviour(Game& game, ObjectBehaviourTrigger trigger) override;

    inline void setChestID(uint16_t chestID) {this->chestID = chestID;}
    inline uint16_t getChestID() {return chestID;}

    // int getChestCapactity();

    void openChest();
    void closeChest();

    // Save / load
    BuildableObjectPOD getPOD() const override;
    void loadFromPOD(const BuildableObjectPOD& pod) override;

private:
    void removeChestFromPool(Game& game);

private:
    uint16_t chestID = 0xFFFF;

};