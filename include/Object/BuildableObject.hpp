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
#include "Player/InventoryData.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"

#include "GUI/InventoryGUI.hpp"

class Game;

// enum ObjectInteractionType
// {
//     NoAction,
//     Chest,
//     Rocket
// };

enum ObjectBehaviourTrigger
{
    RocketFlyUp, RocketFlyDown,
    RocketExit
};

constexpr int DUMMY_OBJECT_COLLISION = -10;
constexpr int DUMMY_OBJECT_NO_COLLISION = -11;

class BuildableObject : public WorldObject
{
public:
    BuildableObject(sf::Vector2f position, ObjectType objectType, bool randomiseAnimation = true);

    virtual BuildableObject* clone();

    virtual void update(Game& game, float dt, bool onWater, bool loopAnimation = true);

    virtual void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, float dt, float gameTime, int worldSize, const sf::Color& color) const override;

    void createLightSource(LightingEngine& lightingEngine, sf::Vector2f topLeftChunkPos) const override;

    // Returns true if destroyed
    virtual bool damage(int amount, Game& game, InventoryData& inventory, bool giveItems = true);
    
    void setWorldPosition(sf::Vector2f position);

    inline ObjectType getObjectType() const {return objectType;}

    inline bool isAlive() {return health > 0;}

    virtual void interact(Game& game);
    virtual bool isInteractable() const;

    virtual void triggerBehaviour(Game& game, ObjectBehaviourTrigger trigger);

    // -- Object reference -- //
    BuildableObject(ObjectReference _objectReference);

    inline bool isObjectReference() const {return objectReference.has_value();}

    inline const std::optional<ObjectReference>& getObjectReference() const {return objectReference;}

    // -- Dummy object -- //
    bool isDummyObject();

    bool dummyHasCollision();

    // Save / load
    virtual BuildableObjectPOD getPOD() const;
    virtual void loadFromPOD(const BuildableObjectPOD& pod);

protected:
    void giveItemDrops(InventoryData& inventory, const std::vector<ItemDrop>& itemDrops);

    void drawObject(sf::RenderTarget& window, SpriteBatch& spriteBatch, float gameTime, int worldSize, const sf::Color& color,
        std::optional<std::vector<sf::IntRect>> textureRectsOverride = std::nullopt, std::optional<sf::Vector2f> textureOriginOverride = std::nullopt) const;

protected:
    ObjectType objectType = 0;
    int health = 1;
    float flash_amount;

    int8_t animationDirection = 1;
    AnimatedTextureMinimal animatedTexture;

    // If reference to a buildable object
    std::optional<ObjectReference> objectReference = std::nullopt;

};