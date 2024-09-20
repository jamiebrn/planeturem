#pragma once

#include <SFML/Graphics.hpp>
#include <steam_api.h>
#include <isteamuserstats.h>
#include <optional>
#include <iostream>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/Sounds.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Camera.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/SpriteBatch.hpp"
#include "Object/WorldObject.hpp"
#include "Object/ObjectReference.hpp"
#include "Player/InventoryData.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"

#include "GUI/FurnaceGUI.hpp"
#include "GUI/InventoryGUI.hpp"

enum ObjectInteraction
{
    NoAction,
    Chest
};

struct ObjectInteractionEventData
{
    ObjectInteraction interactionType;
    uint16_t chestID;
};

class BuildableObject : public WorldObject
{
public:
    BuildableObject(sf::Vector2f position, ObjectType objectType);

    void update(float dt, bool onWater);

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) override;
    void drawGUI(sf::RenderTarget& window, float dt, const sf::Color& color);

    // Returns true if destroyed
    bool damage(int amount, InventoryData& inventory);
    
    ObjectInteractionEventData interact();

    void setWorldPosition(sf::Vector2f position);

    inline ObjectType getObjectType() const {return objectType;}

    inline bool isAlive() {return health > 0;}

    // -- Object reference (blank / filler object) -- //

    BuildableObject(ObjectReference _objectReference);

    inline bool isObjectReference() const {return objectReference.has_value();}

    inline const std::optional<ObjectReference>& getObjectReference() const {return objectReference;}

    // -- Chest -- //

    inline void setChestID(uint16_t chestID) {this->chestID = chestID;}
    inline uint16_t getChestID() {return chestID;}

    int getChestCapactity();

    void openChest();
    void closeChest();


private:
    ObjectType objectType = 0;
    int health = 1;
    float flash_amount;

    int8_t animationDirection = 1;
    AnimatedTextureMinimal animatedTexture;

    uint16_t chestID = 0xFFFF;

    // If reference to a buildable object
    std::optional<ObjectReference> objectReference = std::nullopt;

};