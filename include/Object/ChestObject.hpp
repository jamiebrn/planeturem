#pragma once

#include <SFML/Graphics.hpp>

#include <cstdint>

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"

class ChestObject : public BuildableObject
{
public:
    ChestObject(sf::Vector2f position, ObjectType objectType);

    BuildableObject* clone() override;

    void update(float dt, bool onWater, bool loopAnimation) override;

    ObjectInteractionType interact() const override;

    inline void setChestID(uint16_t chestID) {this->chestID = chestID;}
    inline uint16_t getChestID() {return chestID;}

    int getChestCapactity();

    void openChest();
    void closeChest();

    // Save / load
    BuildableObjectPOD getPOD() const override;
    void loadFromPOD(const BuildableObjectPOD& pod) override;

private:
    uint16_t chestID = 0xFFFF;

};