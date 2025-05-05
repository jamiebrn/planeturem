#pragma once

#include <cmath>
#include <vector>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include <extlib/cereal/archives/binary.hpp>

#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/CollisionCircle.hpp"
#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/TextDraw.hpp"
// #include "Core/SpriteBatch.hpp"

#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ToolData.hpp"
#include "Data/ArmourData.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/ToolDataLoader.hpp"
#include "Data/ArmourDataLoader.hpp"

#include "World/ChunkPosition.hpp"

#include "GUI/ItemSlot.hpp"

#include "Object/WorldObject.hpp"

class ItemPickup : public WorldObject
{
public:
    ItemPickup() = default;
    ItemPickup(pl::Vector2f position, ItemType itemType, float gameTime, int count) : WorldObject(position),
        itemType(itemType), count(count), spawnGameTime(gameTime) {}
    void resetSpawnTime(float gameTime);

    bool isBeingPickedUp(const CollisionRect& playerCollision, float gameTime) const;

    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
        const pl::Color& color) const override;
    
    inline ItemType getItemType() const {return itemType;}
    
    inline uint16_t getItemCount() const {return count;}
    inline void setItemCount(uint16_t count) {this->count = count;}

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(position.x, position.y, itemType, count, spawnGameTime);
    }

private:
    static constexpr float PICKUP_RADIUS = 4.0f;
    static constexpr float SPAWN_FLASH_TIME = 0.4f;

    ItemType itemType;
    uint16_t count;
    float spawnGameTime;

};

struct ItemPickupReference
{
    ChunkPosition chunk;
    uint64_t id = 0;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(chunk.x, chunk.y, id);
    }
};