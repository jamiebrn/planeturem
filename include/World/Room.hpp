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
        // PODs only used for object metadata, e.g. chests etc
        std::vector<std::vector<std::optional<BuildableObjectPOD>>> pods = getObjectPODs();
        archive(roomType, pods);
    }

    template<class Archive>
    void load(Archive& archive, const std::uint32_t version)
    {
        if (version == 1)
        {
            loadingObjectPodsTemp = std::make_unique<std::vector<std::vector<std::optional<BuildableObjectPOD>>>>();

            archive(roomType, *loadingObjectPodsTemp);
        }
        else if (version == 2)
        {
            std::vector<std::vector<std::optional<BuildableObjectPOD>>> podMetadatas;

            archive(roomType, podMetadatas);

            createObjects(nullptr);

            for (int y = 0; y < objectGrid.size(); y++)
            {
                for (int x = 0; x < objectGrid[y].size(); x++)
                {
                    BuildableObject* object = objectGrid[y][x].get();

                    bool usedMetadata = false;

                    if (!podMetadatas[y][x].has_value())
                    {
                        continue;
                    }

                    if (object != nullptr)
                    {
                        usedMetadata = object->injectPODMetadata(podMetadatas[y][x].value());
                    }

                    if (!usedMetadata && podMetadatas[y][x]->chestID != 0xFFFF)
                    {
                        if (unusedMetadataChestIDs == nullptr)
                        {
                            unusedMetadataChestIDs = std::make_unique<std::vector<uint16_t>>();
                        }

                        unusedMetadataChestIDs->push_back(podMetadatas[y][x]->chestID);
                    }
                }
            }

            createCollisionRects();
        }

        // loadObjectPODs();
    }

    // Also initialises objects from pods and therefore creates collisions etc
    // Used in legacy room save handling
    inline void mapVersions(const std::unordered_map<ObjectType, ObjectType>& objectVersionMap)
    {
        if (loadingObjectPodsTemp == nullptr)
        {
            return;
        }

        for (auto& objectRow : *loadingObjectPodsTemp)
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

        loadObjectPODs();

        loadingObjectPodsTemp.reset();
    }

    inline void clearUnusedChestIDs(ChestDataPool& chestDataPool)
    {
        if (unusedMetadataChestIDs == nullptr)
        {
            return;
        }

        for (uint16_t chestID : *unusedMetadataChestIDs)
        {
            chestDataPool.destroyChest(chestID);
        }

        unusedMetadataChestIDs = nullptr;
    }

private:
    void createObjects(ChestDataPool* chestDataPool);
    void setObjectFromBitmask(sf::Vector2i tile, uint8_t bitmaskValue, ChestDataPool* chestDataPool);

    void createCollisionRects();
    
    std::vector<std::vector<std::optional<BuildableObjectPOD>>> getObjectPODs() const;
    void loadObjectPODs();

private:
    RoomType roomType = -1;

    std::vector<CollisionRect> collisionRects;
    std::optional<CollisionRect> warpExitRect;

    // Objects in room
    std::vector<std::vector<std::unique_ptr<BuildableObject>>> objectGrid;

    std::unique_ptr<std::vector<std::vector<std::optional<BuildableObjectPOD>>>> loadingObjectPodsTemp = nullptr;
    std::unique_ptr<std::vector<uint16_t>> unusedMetadataChestIDs = nullptr;

};

CEREAL_CLASS_VERSION(Room, 2);