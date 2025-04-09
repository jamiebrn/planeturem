#pragma once

// #include <SFML/Graphics.hpp>
#include <iostream>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Camera.hpp"
// #include "Core/SpriteBatch.hpp"
#include "Core/TextureManager.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"

#include "Object/WorldObject.hpp"
#include "Object/ObjectReference.hpp"

#include "World/LightingEngine.hpp"

#include "Data/typedefs.hpp"
#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"

#include "Object/StructureObjectPOD.hpp"

#include "GameConstants.hpp"

// Forward declare
class Game;
// class StructureObject;

// struct StructureEnterEvent
// {
//     StructureObject* enteredStructure;
//     // StructureType enteredStructureType;

//     // uint32_t structureID;

//     // Top-left of warp collision / entrance
//     pl::Vector2f entrancePosition;
// };

class StructureObject : public WorldObject
{
public:
    StructureObject(pl::Vector2f position, StructureType structureType);

    void createWarpRect(pl::Vector2f rectPosition);

    bool isPlayerInEntrance(pl::Vector2f playerPos);

    pl::Vector2f getEntrancePosition();

    void setWorldPosition(pl::Vector2f newPosition);

    inline StructureType getStructureType() {return structureType;}

    inline uint32_t getStructureID() {return structureID;}

    inline void setStructureID(uint32_t id) {structureID = id;}

    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const pl::Color& color) const override;

    void createLightSource(LightingEngine& lightingEngine, pl::Vector2f topLeftChunkPos) const override;

    StructureObjectPOD getPOD(pl::Vector2f chunkPosition);
    void loadFromPOD(const StructureObjectPOD& pod, pl::Vector2f chunkPosition);

private:
    StructureType structureType;

    std::optional<CollisionRect> warpEntranceRect = std::nullopt;

    // 0xFFFFFFFF used for uninitialised structure / room
    uint32_t structureID = 0xFFFFFFFF;

};