#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <optional>
#include <iostream>
#include <unordered_map>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/Sounds.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Camera.hpp"
#include "Core/AnimatedTexture.hpp"
// #include "Core/SpriteBatch.hpp"
#include "Object/WorldObject.hpp"
#include "Object/ObjectReference.hpp"
#include "Object/BuildableObjectPOD.hpp"
#include "Object/ParticleSystem.hpp"
#include "Player/InventoryData.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"

#include "GUI/InventoryGUI.hpp"
#include "GUI/HitMarkers.hpp"

class Game;
class ChunkManager;

enum ObjectBehaviourTrigger
{
    RocketFlyUp, RocketFlyDown,
    RocketExit,

    ChestOpen, ChestClose
};

constexpr int DUMMY_OBJECT_COLLISION = -10;
constexpr int DUMMY_OBJECT_NO_COLLISION = -11;

class BuildableObject : public WorldObject
{
public:
    BuildableObject(pl::Vector2f position, ObjectType objectType, bool randomiseAnimation = true);

    virtual BuildableObject* clone();

    virtual void update(Game& game, float dt, bool onWater, bool loopAnimation = true);

    virtual void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
        const pl::Color& color) const override;

    void createLightSource(LightingEngine& lightingEngine, pl::Vector2f topLeftChunkPos) const override;

    // Returns true if destroyed
    virtual bool damage(int amount, Game& game, ChunkManager& chunkManager, ParticleSystem& particleSystem, bool giveItems = true);

    void createHitParticles(ParticleSystem& particleSystem);

    void createHitMarker(int amount);
    
    void setWorldPosition(pl::Vector2f position);

    inline ObjectType getObjectType() const {return objectType;}

    inline bool isAlive() {return health > 0;}

    virtual void interact(Game& game, bool isClient);
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

    virtual inline bool injectPODMetadata(const BuildableObjectPOD& pod) {return false;}

    void mapVersions(const std::unordered_map<ObjectType, ObjectType>& objectVersionMap)
    {
        if (isDummyObject() || isObjectReference())
        {
            return;
        }
        
        objectType = objectVersionMap.at(objectType);
    }

protected:
    void createItemPickups(ChunkManager& chunkManager, Game& game, const std::vector<ItemDrop>& itemDrops, float gameTime, bool alertGame = true);

    void drawObject(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, float gameTime, int worldSize, const pl::Color& color,
        std::optional<std::vector<pl::Rect<int>>> textureRectsOverride = std::nullopt, std::optional<pl::Vector2f> textureOriginOverride = std::nullopt,
        const pl::Texture* textureOverride = nullptr) const;

protected:
    ObjectType objectType = 0;
    int health = 1;
    float flash_amount;

    int8_t animationDirection = 1;
    AnimatedTextureMinimal animatedTexture;

    // If reference to a buildable object
    std::optional<ObjectReference> objectReference = std::nullopt;

};