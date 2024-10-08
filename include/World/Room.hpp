#pragma once

#include <SFML/Graphics.hpp>

#include <vector>
#include <array>
#include <optional>
#include <memory>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/optional.hpp>

#include "Core/CollisionRect.hpp"
#include "Core/TextureManager.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/BuildableObjectFactory.hpp"
#include "Object/ChestObject.hpp"

#include "Object/BuildableObjectPOD.hpp"

#include "Player/InventoryData.hpp"
#include "World/ChestDataPool.hpp"

#include "Data/StructureData.hpp"

#include "GameConstants.hpp"
#include "DebugOptions.hpp"

class Room
{
public:
    Room();
    Room(const RoomData& roomData, ChestDataPool& chestDataPool);

    // Copying
    Room(const Room& room);
    Room& operator=(const Room& room);

    bool handleStaticCollisionX(CollisionRect& collisionRect, float dx) const;
    bool handleStaticCollisionY(CollisionRect& collisionRect, float dy) const;

    bool isPlayerInExit(sf::Vector2f playerPos) const;

    sf::Vector2f getEntrancePosition() const;

    std::vector<const WorldObject*> getObjects() const;

    std::vector<std::vector<std::unique_ptr<BuildableObject>>>& getObjectGrid() {return objectGrid;}

    void updateObjects(float dt);

    BuildableObject* getObject(sf::Vector2f mouseWorldPos);
    BuildableObject* getObject(sf::Vector2i tile);

    sf::Vector2i getSelectedTile(sf::Vector2f mouseWorldPos);

    void draw(sf::RenderTarget& window) const;


    // Save / load
    template<class Archive>
    void save(Archive& archive) const
    {
        std::vector<std::vector<std::optional<BuildableObjectPOD>>> pods = getObjectPODs();
        archive(roomData, pods);
    }

    template<class Archive>
    void load(Archive& archive)
    {
        std::vector<std::vector<std::optional<BuildableObjectPOD>>> pods;
        
        archive(roomData, pods);

        loadObjectPODs(pods);
    }

private:
    void createObjects(ChestDataPool& chestDataPool);

    void createCollisionRects();
    
    std::vector<std::vector<std::optional<BuildableObjectPOD>>> getObjectPODs() const;
    void loadObjectPODs(const std::vector<std::vector<std::optional<BuildableObjectPOD>>>& pods);

private:
    RoomData roomData;

    std::vector<CollisionRect> collisionRects;
    CollisionRect warpExitRect;

    // Objects in room
    std::vector<std::vector<std::unique_ptr<BuildableObject>>> objectGrid;

};