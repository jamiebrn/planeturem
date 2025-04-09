#pragma once

// #include <SFML/Graphics.hpp>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include <cstdint>

#include "Core/Random.hpp"
#include "Core/Camera.hpp"
#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"

#include "World/DayCycleManager.hpp"

class Game;
class ChunkManager;

class PlantObject : public BuildableObject
{
public:
    PlantObject(pl::Vector2f position, ObjectType objectType, Game& game, const ChunkManager* chunkManager = nullptr, bool randomiseAge = false);

    BuildableObject* clone() override;

    void update(Game& game, float dt, bool onWater, bool loopAnimation) override;

    bool damage(int amount, Game& game, ChunkManager& chunkManager, ParticleSystem& particleSystem, bool giveItems = true) override;

    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const sf::Color& color) const override;

    // Save / load
    BuildableObjectPOD getPOD() const override;
    void loadFromPOD(const BuildableObjectPOD& pod) override;

    virtual bool injectPODMetadata(const BuildableObjectPOD& pod) override;

private:
    int getPlantStage(int currentDay) const;
    const PlantStageObjectData* getPlantStageData(int currentDay) const;

    void changePlantStage(int newStage);

    int getTotalGrowthDays() const;

private:
    int dayPlanted = 0;
    int currentStage = 0;

};