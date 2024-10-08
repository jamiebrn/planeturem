#pragma once

#include <SFML/Graphics.hpp>
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
#include "Object/BuildableObjectPOD.hpp"
#include "Object/ParticleSystem.hpp"
#include "Player/InventoryData.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"

#include "GUI/InventoryGUI.hpp"

enum ObjectInteractionType
{
    NoAction,
    Chest,
    Rocket
};

constexpr int DUMMY_OBJECT_COLLISION = -10;
constexpr int DUMMY_OBJECT_NO_COLLISION = -11;

class BuildableObject : public WorldObject
{
public:
    BuildableObject(sf::Vector2f position, ObjectType objectType, bool randomiseAnimation = true);

    virtual BuildableObject* clone();

    virtual void update(float dt, bool onWater, bool loopAnimation = true);

    virtual void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) const override;

    // Returns true if destroyed
    bool damage(int amount, InventoryData& inventory);
    
    void setWorldPosition(sf::Vector2f position);

    inline ObjectType getObjectType() const {return objectType;}

    inline bool isAlive() {return health > 0;}


    virtual ObjectInteractionType interact() const;
    bool isInteractable() const;


    BuildableObject(ObjectReference _objectReference);

    inline bool isObjectReference() const {return objectReference.has_value();}

    inline const std::optional<ObjectReference>& getObjectReference() const {return objectReference;}

    // -- Dummy object -- //
    
    bool isDummyObject();

    bool dummyHasCollision();

    // Save / load

    virtual BuildableObjectPOD getPOD() const;
    virtual void loadFromPOD(const BuildableObjectPOD& pod);

// private:
//     void drawRocket(sf::RenderTarget& window, SpriteBatch& spriteBatch, const sf::Color& color) const;

protected:
    ObjectType objectType = 0;
    int health = 1;
    float flash_amount;

    int8_t animationDirection = 1;
    AnimatedTextureMinimal animatedTexture;

    // uint16_t chestID = 0xFFFF;

    // float rocketYOffset = 0.0f;

    // If reference to a buildable object
    std::optional<ObjectReference> objectReference = std::nullopt;

};