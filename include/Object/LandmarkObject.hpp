#pragma once

#include <vector>

#include <Graphics/VertexArray.hpp>
#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Shaders.hpp"

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/BuildableObjectPOD.hpp"

#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"

class Game;

class LandmarkObject : public BuildableObject
{
public:
    LandmarkObject(pl::Vector2f position, ObjectType objectType, Game& game, bool placedByThisPlayer, bool flash = false);

    BuildableObject* clone() override;

    virtual void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
        const pl::Color& color) const override;
    
    bool damage(int amount, Game& game, ChunkManager& chunkManager, ParticleSystem& particleSystem, bool giveItems = true) override;

    virtual void interact(Game& game, bool isClient) override;
    virtual bool isInteractable() const override;

    void setLandmarkColour(const pl::Color& colourA, const pl::Color& colourB);
    const pl::Color& getColorA() const;
    const pl::Color& getColorB() const;

    virtual BuildableObjectPOD getPOD() const override;
    virtual void loadFromPOD(const BuildableObjectPOD& pod) override;

private:
    pl::Color colorA, colorB;

};