#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/CollisionCircle.hpp"
#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"
#include "Core/SpriteBatch.hpp"

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
    ItemPickup(sf::Vector2f position, ItemType itemType, float gameTime) : WorldObject(position), itemType(itemType), spawnGameTime(gameTime) {}

    bool isBeingPickedUp(const CollisionRect& playerCollision, float gameTime);

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
        const sf::Color& color) const override;
    
    inline ItemType getItemType() {return itemType;}

private:
    static constexpr float PICKUP_RADIUS = 4.0f;
    static constexpr float SPAWN_FLASH_TIME = 0.4f;

    ItemType itemType;
    float spawnGameTime;

};

struct ItemPickupReference
{
    ItemPickup itemPickup;
    ChunkPosition chunk;
    std::vector<ItemPickup>::iterator pickupIter;
};