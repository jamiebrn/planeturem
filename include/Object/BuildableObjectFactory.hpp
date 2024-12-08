#pragma once

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/ChestObject.hpp"
#include "Object/RocketObject.hpp"
#include "Object/PlantObject.hpp"
#include "Object/NPCObject.hpp"
#include "Object/LandmarkObject.hpp"

class Game;

class BuildableObjectFactory
{
    BuildableObjectFactory() = delete;
public:
    static std::unique_ptr<BuildableObject> create(sf::Vector2f position, ObjectType objectType, Game* game = nullptr, bool placedByPlayer = false);
};