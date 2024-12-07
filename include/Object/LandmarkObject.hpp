#pragma once

#include <SFML/Graphics.hpp>

#include "Core/Shaders.hpp"

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"

#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"

class Game;

class LandmarkObject : public BuildableObject
{
public:
    LandmarkObject(sf::Vector2f position, ObjectType objectType, Game& game);

    BuildableObject* clone() override;

    virtual void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
        const sf::Color& color) const override;

    bool damage(int amount, Game& game, InventoryData& inventory, bool giveItems = true) override;

};