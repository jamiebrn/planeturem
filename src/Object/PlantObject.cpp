#include "Object/PlantObject.hpp"
#include "Game.hpp"
#include "World/ChunkManager.hpp"

PlantObject::PlantObject(pl::Vector2f position, ObjectType objectType, Game& game, const ChunkManager* chunkManager, bool randomiseAge)
    : BuildableObject(position, objectType, false)
{
    int currentDay = game.getDayCycleManager().getCurrentDay();

    if (randomiseAge)
    {
        // Set seed for randgen
        unsigned long int chunkSeed = (game.getPlanetSeed() + chunkManager->getPlanetType()) ^ getChunkInside(chunkManager->getWorldSize()).hash();
        pl::Vector2<int> tile = getChunkTileInside(position, chunkManager->getWorldSize());
        chunkSeed |= tile.x | tile.y;

        RandInt randGen(chunkSeed);
        
        dayPlanted = currentDay - randGen.generate(0, getTotalGrowthDays());
    }
    else
    {
        dayPlanted = currentDay;
    }

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
    flash_amount = std::max(flash_amount - dt * 3, 0.0f);

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
    const PlantStageObjectData& plantStageData = objectData.plantStageObjectData->at(currentStage);

    animatedTexture.update(dt, animationDirection, plantStageData.textureRects.size(), objectData.textureFrameDelay, loopAnimation);

    int newStage = getPlantStage(game.getDayCycleManager().getCurrentDay());

    if (newStage != currentStage)
    {
        changePlantStage(newStage);
    }
}

void PlantObject::changePlantStage(int newStage)
{
    currentStage = newStage;

    // Reset health to new stage health
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
    health = objectData.plantStageObjectData->at(currentStage).health;
}

bool PlantObject::damage(int amount, Game& game, ChunkManager& chunkManager, ParticleSystem& particleSystem, bool giveItems)
{
    bool destroyed = BuildableObject::damage(amount, game, chunkManager, particleSystem, false);

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
        if (giveItems)
        {
            createItemPickups(chunkManager, game, plantStageData->itemDrops, game.getGameTime());
        }

        return true;
    }

    return false;
}

void PlantObject::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const pl::Color& color) const
{
    // int currentDay = game.getDayCycleManager().getCurrentDay();

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    const PlantStageObjectData& plantStageData = objectData.plantStageObjectData->at(currentStage);

    BuildableObject::drawObject(window, spriteBatch, camera, gameTime, worldSize, color, plantStageData.textureRects, plantStageData.textureOrigin);
}

int PlantObject::getPlantStage(int currentDay) const
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
    int age = currentDay - dayPlanted;

    for (int i = 0; i < objectData.plantStageObjectData->size(); i++)
    {
        const PlantStageObjectData& plantStageData = objectData.plantStageObjectData->at(i);

        // If is final stage, return it as current stage
        if (i == objectData.plantStageObjectData->size() - 1)
        {
            return i;
        }

        // Check age is in bounds of plant stage
        if (age >= plantStageData.minDay && age <= plantStageData.maxDay)
        {
            return i;
        }
    }

    // Default case
    return -1;
}

const PlantStageObjectData* PlantObject::getPlantStageData(int currentDay) const
{
    int stage = getPlantStage(currentDay);
    if (stage < 0)
    {
        return nullptr;
    }

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    return &objectData.plantStageObjectData->at(stage);
}

int PlantObject::getTotalGrowthDays() const
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
    int totalDays = 0;

    for (const PlantStageObjectData& plantStageData : objectData.plantStageObjectData.value())
    {
        totalDays += plantStageData.maxDay - plantStageData.minDay + 1;
    }

    return totalDays;
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

bool PlantObject::injectPODMetadata(const BuildableObjectPOD& pod)
{
    dayPlanted = pod.plantDayPlanted;
    return true;
}