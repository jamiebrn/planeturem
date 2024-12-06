#include "Object/LandmarkObject.hpp"
#include "Game.hpp"

LandmarkObject::LandmarkObject(sf::Vector2f position, ObjectType objectType, Game& game)
    : BuildableObject(position, objectType)
{

}

BuildableObject* LandmarkObject::clone()
{
    return new LandmarkObject(*this);
}

void LandmarkObject::update(Game& game, float dt, bool onWater, bool loopAnimation)
{
    BuildableObject::update(game, dt, onWater, loopAnimation);
}

bool LandmarkObject::damage(int amount, Game& game, InventoryData& inventory, bool giveItems)
{
    bool destroyed = BuildableObject::damage(amount, game, inventory);

    if (destroyed)
    {
        // Alert game that landmark has been destroyed
    }

    return destroyed;
}