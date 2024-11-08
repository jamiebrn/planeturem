#pragma once

#include <SFML/Graphics.hpp>

#include <cstdint>

#include "Core/Random.hpp"
#include "Core/Camera.hpp"
#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"

#include "World/DayCycleManager.hpp"

class Game;

class PlantObject : public BuildableObject
{
public:
    PlantObject(sf::Vector2f position, ObjectType objectType, Game& game, bool randomiseAge = false);

    BuildableObject* clone() override;

    void update(Game& game, float dt, bool onWater, bool loopAnimation) override;

    bool damage(int amount, Game& game, InventoryData& inventory, bool giveItems = true) override;

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const sf::Color& color) const override;

    // Save / load
    BuildableObjectPOD getPOD() const override;
    void loadFromPOD(const BuildableObjectPOD& pod) override;

private:
    int getPlantStage(int currentDay) const;
    const PlantStageObjectData* getPlantStageData(int currentDay) const;

    void changePlantStage(int newStage);

    int getTotalGrowthDays() const;

private:
    int dayPlanted = 0;
    int currentStage = 0;

};