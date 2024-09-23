#pragma once

#include <SFML/Graphics.hpp>

#include <vector>
#include <array>

#include "Core/CollisionRect.hpp"
#include "Core/TextureManager.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"

#include "Data/StructureData.hpp"

#include "GameConstants.hpp"
#include "DebugOptions.hpp"

class Room
{
public:
    Room(const RoomData& roomData);

    bool handleStaticCollisionX(CollisionRect& collisionRect, float dx) const;
    bool handleStaticCollisionY(CollisionRect& collisionRect, float dy) const;

    bool isPlayerInExit(sf::Vector2f playerPos) const;

    sf::Vector2f getEntrancePosition() const;

    void draw(sf::RenderTarget& window) const;

private:
    void createCollisionRects();

private:
    RoomData roomData;

    std::vector<CollisionRect> collisionRects;
    CollisionRect warpExitRect;

};