#pragma once

#include <SFML/Graphics.hpp>

#include "Core/Camera.hpp"
#include "Core/SpriteBatch.hpp"
#include "Core/TextureManager.hpp"
#include "Core/ResolutionHandler.hpp"

#include "Object/WorldObject.hpp"

#include "Data/typedefs.hpp"
#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"

class StructureObject : public WorldObject
{
public:
    StructureObject(sf::Vector2f position, StructureType structureType);

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) override;

private:
    StructureType structureType;

};