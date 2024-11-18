#pragma once

#include <SFML/Graphics.hpp>

#include <vector>
#include <array>
#include <optional>
#include <memory>
#include <unordered_map>

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
#include "Data/StructureDataLoader.hpp"

#include "GameConstants.hpp"
#include "DebugOptions.hpp"

class Game;

class Room
{
public:
    Room();
    Room(RoomType roomType, ChestDataPool& chestDataPool);

    // Copying
    Room(const Room& room);
    Room& operator=(const Room& room);

    bool handleStaticCollisionX(CollisionRect& collisionRect, float dx) const;
    bool handleStaticCollisionY(CollisionRect& collisionRect, float dy) const;

    bool isPlayerInExit(sf::Vector2f playerPos) const;

    std::optional<sf::Vector2f> getEntrancePosition() const;

    std::vector<const WorldObject*> getObjects() const;

    std::vector<std::vector<std::unique_ptr<BuildableObject>>>& getObjectGrid() {return objectGrid;}

    void updateObjects(Game& game, float dt);

    // BuildableObject* getObject(sf::Vector2f mouseWorldPos) const;
    BuildableObject* getObject(sf::Vector2i tile) const;

    bool getFirstRocketObjectReference(ObjectReference& objectReference) const;

    RoomType getRoomType() const;

    void draw(sf::RenderTarget& window, const Camera& camera) const;


    // Save / load
    template<class Archive>
    void save(Archive& archive, const std::uint32_t version) const
    {
        std::vector<std::vector<std::optional<BuildableObjectPOD>>> pods = getObjectPODs();
        archive(roomType, pods);
    }

    template<class Archive>
    void load(Archive& archive, const std::uint32_t version)
    {
        std::vector<std::vector<std::optional<BuildableObjectPOD>>> pods;
        
        archive(roomType, pods);

        loadObjectPODs(pods);
    }

    void mapVersions(const std::unordered_map<ObjectType, ObjectType>& objectVersionMap)
    {
        for (auto& objectRow : objectGrid)
        {
            for (auto& object : objectRow)
            {
                if (!object)
                {
                    continue;
                }

                object->mapVersions(objectVersionMap);
            }
        }   
    }

private:
    void createObjects(ChestDataPool& chestDataPool);
    void setObjectFromBitmask(sf::Vector2i tile, uint8_t bitmaskValue, ChestDataPool& chestDataPool);

    void createCollisionRects();
    
    std::vector<std::vector<std::optional<BuildableObjectPOD>>> getObjectPODs() const;
    void loadObjectPODs(const std::vector<std::vector<std::optional<BuildableObjectPOD>>>& pods);

private:
    RoomType roomType = -1;

    std::vector<CollisionRect> collisionRects;
    std::optional<CollisionRect> warpExitRect;

    // Objects in room
    std::vector<std::vector<std::unique_ptr<BuildableObject>>> objectGrid;

};