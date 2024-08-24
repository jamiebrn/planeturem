#pragma once

#include <SFML/Graphics.hpp>
#include <steam_api.h>
#include <isteamuserstats.h>
#include <optional>
#include <iostream>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Camera.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Object/WorldObject.hpp"
#include "Object/ObjectReference.hpp"
#include "Player/Inventory.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"

#include "GUI/FurnaceGUI.hpp"

enum ObjectInteraction
{
    NoAction,
    OpenFurnace
};

struct ObjectInteractionEventData
{
    ObjectInteraction interactionType;
    uint64_t objectID;
};

class BuildableObject : public WorldObject
{
public:
    BuildableObject(sf::Vector2f position, ObjectType objectType);

    void update(float dt);

    void draw(sf::RenderTarget& window, float dt, float gameTime, const sf::Color& color) override;
    void drawGUI(sf::RenderTarget& window, float dt, const sf::Color& color);

    void damage(int amount);
    ObjectInteractionEventData interact();

    void setWorldPosition(sf::Vector2f position);

    inline ObjectType getObjectType() const {return objectType;}

    inline bool isAlive() {return health > 0;}

    // When used as a reference to another object
    BuildableObject(ObjectReference _objectReference);

    inline bool isObjectReference() const {return objectReference.has_value();}

    inline const std::optional<ObjectReference>& getObjectReference() const {return objectReference;}

private:
    ObjectType objectType = 0;
    int health = 0;
    float flash_amount;

    AnimatedTextureMinimal animatedTexture;

    uint64_t furnaceID = 0;

    // If reference to a buildable object
    std::optional<ObjectReference> objectReference = std::nullopt;

};