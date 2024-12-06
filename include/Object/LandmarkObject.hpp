#pragma once

#include <SFML/Graphics.hpp>

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"

class Game;

class LandmarkObject : public BuildableObject
{
public:
    LandmarkObject(sf::Vector2f position, ObjectType objectType, Game& game);

    BuildableObject* clone() override;

    void update(Game& game, float dt, bool onWater, bool loopAnimation) override;

    bool damage(int amount, Game& game, InventoryData& inventory, bool giveItems = true) override;
};