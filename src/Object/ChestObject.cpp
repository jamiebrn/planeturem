#include "Object/ChestObject.hpp"
#include "Game.hpp"

ChestObject::ChestObject(pl::Vector2f position, ObjectType objectType, const BuildableObjectCreateParameters& parameters)
    : BuildableObject(position, objectType, parameters)
{
    animatedTexture.setFrame(0);
    closeChest();
}

BuildableObject* ChestObject::clone()
{
    return new ChestObject(*this);
}

void ChestObject::update(Game& game, float dt, bool onWater, bool loopAnimation)
{
    BuildableObject::update(game, dt, onWater, false);

    // if (game.getOpenChestID() != chestID)
    // {
    //     closeChest();
    // }
}

bool ChestObject::damage(int amount, Game& game, ChunkManager& chunkManager, ParticleSystem& particleSystem, bool giveItems)
{
    // if (chestID != 0xFFFF)
    // {
    //     if (!game.getChestDataPool().getChestDataPtr(chestID)->isEmpty())
    //     {
    //         amount = 0;
    //     }
    // }

    bool destroyed = BuildableObject::damage(amount, game, chunkManager, particleSystem, giveItems);

    if (destroyed)
    {
        InventoryData* chestData = game.getChestDataPool().getChestDataPtr(chestID);
        if (chestData != nullptr)
        {
            // Create pickups for items in chest
            if (!chestData->isEmpty())
            {
                std::vector<ItemDrop> itemDrops;
                for (auto& itemCount : chestData->getData())
                {
                    if (!itemCount.has_value())
                    {
                        continue;
                    }

                    ItemDrop itemDrop;
                    itemDrop.item = itemCount->first;
                    itemDrop.chance = 1.0f;
                    itemDrop.minAmount = itemCount->second;
                    itemDrop.maxAmount = itemCount->second;

                    itemDrops.push_back(itemDrop);
                }

                if (giveItems)
                {
                    BuildableObject::createItemPickups(chunkManager, game, itemDrops, game.getGameTime()); 
                }
            }
        }

        removeChestFromPool(game);

        if (game.getOpenChestID() == chestID)
        {
            game.closeChest();
        }
    }

    return destroyed;
}

void ChestObject::interact(Game& game, bool isClient)
{
    // Chest interaction stuff
    if (!isClient)
    {
        if (chestID == 0xFFFF)
        {
            const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
            chestID = game.getChestDataPool().createChest(objectData.chestCapacity);
        }
    
        // If chestID is still 0xFFFF, then max chest number has been reached
        if (chestID == 0xFFFF)
        {
            return;
        }

        // Animation
        // openChest();
    }

    // Close chest if already open
    if (chestID != 0xFFFF)
    {
        if (game.getOpenChestID() == chestID)
        {
            game.closeChest();
            return;
        }
    }

    game.openChest(*this, std::nullopt, isClient);
}

bool ChestObject::isInteractable() const
{
    return true;
}

uint16_t ChestObject::createChestID(Game& game, std::optional<LocationState> locationState)
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
    chestID = game.getChestDataPool(locationState).createChest(objectData.chestCapacity);
    return chestID;
}

// int ChestObject::getChestCapactity()
// {
//     if (objectType < 0)
//         return 0;

//     const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

//     return objectData.chestCapacity;
// }

void ChestObject::openChest()
{
    // Play open chest animation
    animationDirection = 1;
}

void ChestObject::closeChest()
{
    // Rewind open chest animation
    animationDirection = -1;
}

bool ChestObject::isOpen()
{
    return (animationDirection > 0);
}

void ChestObject::removeChestFromPool(Game& game)
{
    game.getChestDataPool().destroyChest(chestID);
}

// Save / load
BuildableObjectPOD ChestObject::getPOD() const
{
    BuildableObjectPOD pod = BuildableObject::getPOD();
    pod.chestID = chestID;
    return pod;
}

void ChestObject::loadFromPOD(const BuildableObjectPOD& pod)
{
    BuildableObject::loadFromPOD(pod);
    chestID = pod.chestID;
}

bool ChestObject::injectPODMetadata(const BuildableObjectPOD& pod)
{
    chestID = pod.chestID;
    return true;
}