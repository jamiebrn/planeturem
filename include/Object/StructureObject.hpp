#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>

#include "Core/Camera.hpp"
#include "Core/SpriteBatch.hpp"
#include "Core/TextureManager.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"

#include "Object/WorldObject.hpp"

#include "Data/typedefs.hpp"
#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"

#include "GameConstants.hpp"

class StructureObject : public WorldObject
{
public:
    StructureObject(sf::Vector2f position, StructureType structureType);

    void createWarpRect(sf::Vector2f rectPosition);

    bool isPlayerInEntrance(sf::Vector2f playerPos);

    void setWorldPosition(sf::Vector2f newPosition);

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) override;

private:
    StructureType structureType;

    std::optional<CollisionRect> warpEntranceRect = std::nullopt;

    // 0xFFFFFFFF used for uninitialised room
    uint32_t roomID = 0xFFFFFFFF;

};