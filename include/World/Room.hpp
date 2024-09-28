#pragma once

#include <SFML/Graphics.hpp>

#include <vector>
#include <array>
#include <optional>

#include "Core/CollisionRect.hpp"
#include "Core/TextureManager.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"

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

    std::vector<const WorldObject*> getObjects() const;

    const std::vector<std::vector<std::optional<BuildableObject>>>& getObjectGrid() const {return objectGrid;}

    void updateObjects(float dt);

    BuildableObject* getObject(sf::Vector2f mouseWorldPos);
    BuildableObject* getObject(sf::Vector2i tile);

    sf::Vector2i getSelectedTile(sf::Vector2f mouseWorldPos);

    void draw(sf::RenderTarget& window) const;

private:
    void createObjects();

    void createCollisionRects();

private:
    RoomData roomData;

    std::vector<CollisionRect> collisionRects;
    CollisionRect warpExitRect;

    // Objects in room
    std::vector<std::vector<std::optional<BuildableObject>>> objectGrid;

};