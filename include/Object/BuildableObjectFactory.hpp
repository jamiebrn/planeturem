#pragma once

#include <Vector.hpp>

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/ChestObject.hpp"
#include "Object/RocketObject.hpp"
#include "Object/PlantObject.hpp"
#include "Object/NPCObject.hpp"
#include "Object/LandmarkObject.hpp"

class Game;
class ChunkManager;

namespace BuildableObjectFactory
{

std::unique_ptr<BuildableObject> create(pl::Vector2f position, ObjectType objectType, Game* game = nullptr, bool placedByPlayer = false,
    bool placedByThisPlayer = false, ChunkManager* chunkManager = nullptr);

}