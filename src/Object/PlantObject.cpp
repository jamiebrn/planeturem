#include "Object/PlantObject.hpp"
#include "Game.hpp"

PlantObject::PlantObject(sf::Vector2f position, ObjectType objectType, Game& game, bool randomiseAge)
    : BuildableObject(position, objectType, false)
{
    int currentDay = game.getDayCycleManager().getCurrentDay();

    // Set health
    const PlantStageObjectData* plantStageData = getPlantStageData(currentDay);

    if (plantStageData)
    {
        health = plantStageData->health;
    }
}

BuildableObject* PlantObject::clone()
{
    return new PlantObject(*this);
}

void PlantObject::update(Game& game, float dt, bool onWater, bool loopAnimation)
{
    BuildableObject::update(game, dt, onWater, loopAnimation);
}

bool PlantObject::damage(int amount, Game& game, InventoryData& inventory, bool giveItems)
{
    bool destroyed = BuildableObject::damage(amount, game, inventory, false);

    if (destroyed)
    {
        // Give items based on current growth stage
        const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
        if (!objectData.plantStageObjectData.has_value())
        {
            return true;
        }

        int currentDay = game.getDayCycleManager().getCurrentDay();

        const PlantStageObjectData* plantStageData = getPlantStageData(currentDay);

        if (!plantStageData)
        {
            return true;
        }

        // Give item drops
        giveItemDrops(inventory, plantStageData->itemDrops);

        return true;
    }

    return false;
}

void PlantObject::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, float dt, float gameTime, int worldSize, const sf::Color& color) const
{
    int currentDay = game.getDayCycleManager().getCurrentDay();

    const PlantStageObjectData* plantStageData = getPlantStageData(currentDay);

    if (!plantStageData)
    {
        return;
    }

    BuildableObject::drawObject(window, spriteBatch, gameTime, worldSize, color, plantStageData->textureRects, plantStageData->textureOrigin);
}

const PlantStageObjectData* PlantObject::getPlantStageData(int currentDay) const
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
    int age = currentDay - dayPlanted;

    for (int i = 0; i < objectData.plantStageObjectData->size(); i++)
    {
        const PlantStageObjectData& plantStageData = objectData.plantStageObjectData->at(i);

        // If is final stage, return it as current stage
        if (i == objectData.plantStageObjectData->size() - 1)
        {
            return &plantStageData;
        }

        // Check age is in bounds of plant stage
        if (age >= plantStageData.minDay && age <= plantStageData.maxDay)
        {
            return &plantStageData;
        }
    }

    return nullptr;
}

// Save / load
BuildableObjectPOD PlantObject::getPOD() const
{
    BuildableObjectPOD pod = BuildableObject::getPOD();
    pod.plantDayPlanted = dayPlanted;
    return pod;
}

void PlantObject::loadFromPOD(const BuildableObjectPOD& pod)
{
    BuildableObject::loadFromPOD(pod);
    dayPlanted = pod.plantDayPlanted;
}