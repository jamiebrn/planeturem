#include "Object/LandmarkObject.hpp"
#include "Game.hpp"

LandmarkObject::LandmarkObject(sf::Vector2f position, ObjectType objectType, Game& game)
    : BuildableObject(position, objectType)
{
    game.landmarkPlaced(*this);
}

BuildableObject* LandmarkObject::clone()
{
    return new LandmarkObject(*this);
}

bool LandmarkObject::damage(int amount, Game& game, InventoryData& inventory, bool giveItems)
{
    bool destroyed = BuildableObject::damage(amount, game, inventory);

    if (destroyed)
    {
        game.landmarkDestroyed(*this);
    }

    return destroyed;
}